/************************************************************************
 *   IRC - Internet Relay Chat, src/m_gline.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers. 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  $Id: m_gline.c,v 1.9 2000/12/08 08:19:56 db Exp $
 */
#include "tools.h"
#include "handlers.h"
#include "m_gline.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "config.h"
#include "dline_conf.h"
#include "irc_string.h"
#include "ircd.h"
#include "m_kline.h"
#include "mtrie_conf.h"
#include "numeric.h"
#include "fdlist.h"
#include "s_bsd.h"
#include "s_conf.h"
#include "s_misc.h"
#include "scache.h"
#include "send.h"
#include "msg.h"
#include "fileio.h"
#include "s_serv.h"
#include "hash.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>


/* external variables */
extern ConfigFileEntryType ConfigFileEntry; /* defined in ircd.c */

/* internal variables */
extern struct ConfItem *glines;
static GLINE_PENDING *pending_glines;

/* internal functions */
void set_local_gline(
		     const char *oper_nick,
		     const char *oper_user,
		     const char *oper_host,
		     const char *oper_server,
		     const char *user,
		     const char *host,
		     const char *reason);
void add_gline(struct ConfItem *);
void log_gline_request(const char*,const char*,const char*,
		       const char* oper_server,
		       const char *,const char *,const char *);

void log_gline(struct Client *,GLINE_PENDING *,
	       const char *, const char *,const char *,
	       const char* oper_server,
	       const char *,const char *,const char *);


void expire_pending_glines();

void
check_majority_gline(struct Client *sptr,
		     const char *oper_nick, const char *oper_user,
		     const char *oper_host, const char *oper_server,
		     const char *user, const char *host, const char *reason);

int majority_gline(struct Client *sptr,
		   const char *oper_nick, const char *oper_username,
		   const char *oper_host, 
		   const char *oper_server,
		   const char *user,
		   const char *host,
		   const char *reason); 

struct Message gline_msgtab = {
    MSG_GLINE, 0, 1, MFLG_SLOW, 0,
      {m_unregistered, m_not_oper, ms_gline, mo_gline}
};

void
_modinit(void)
{
    mod_add_cmd(MSG_GLINE, &gline_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(MSG_GLINE);
}

char *_version = "20001122";

/*
 * mo_gline()
 *
 * inputs       - The usual for a m_ function
 * output       -
 * side effects -
 *
 * Place a G line if 3 opers agree on the identical user@host
 * 
 */
/* Allow this server to pass along GLINE if received and
 * GLINES is not defined.
 */

int mo_gline(struct Client *cptr,
	     struct Client *sptr,
	     int parc,
	     char *parv[])
{
  char *user = NULL;
  char *host = NULL;	              /* user and host of GLINE "victim" */
  const char *reason = NULL;          /* reason for "victims" demise */
  char *p;
  char tmpch;
  int nonwild;
  char tempuser[USERLEN + 2];
  char temphost[HOSTLEN + 1];

  if (ConfigFileEntry.glines)
    {
      /* Only globals can apply Glines */
      if (!IsGlobalOper(sptr))
	{
	  sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
	  return 0;
	}

      if (!IsSetOperGline(sptr))
	{
	  sendto_one(sptr,":%s NOTICE %s :You have no G flag",me.name,parv[0]);
	  return 0;
	}
			
      if ( parc < 3 )
	{
	  sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
		     me.name, parv[0], "GLINE");
	  return 0;
	}
			
      if ( (host = strchr(parv[1], '@')) || *parv[1] == '*' )
	{
	  /* Explicit user@host mask given */
	      
	  if(host)                      /* Found user@host */
	    {
	      user = parv[1];   /* here is user part */
	      *(host++) = '\0'; /* and now here is host */
	    }
	  else
	    {
	      user = "*";               /* no @ found, assume its *@somehost */
	      host = parv[1];
	    }
	      
	  if (!*host)           /* duh. no host found, assume its '*' host */
	    host = "*";
	      
	  strncpy_irc(tempuser, user, USERLEN + 1);     /* allow for '*' */
	  tempuser[USERLEN + 1] = '\0';
	  strncpy_irc(temphost, host, HOSTLEN);
	  temphost[HOSTLEN] = '\0';
	  user = tempuser;
	  host = temphost;
	}
      else
	{
	  sendto_one(sptr, ":%s NOTICE %s :Can't G-Line a nick use user@host",
		     me.name,
		     parv[0]);
	  return 0;
	}
			
      if(strchr(parv[2], ':'))
	{
	  sendto_one(sptr,
		     ":%s NOTICE %s :Invalid character ':' in comment",
		     me.name, parv[2]);
	  return 0;
	}
	  
      /*
       * Now we must check the user and host to make sure there
       * are at least NONWILDCHARS non-wildcard characters in
       * them, otherwise assume they are attempting to gline
       * *@* or some variant of that. This code will also catch
       * people attempting to gline *@*.tld, as long as NONWILDCHARS
       * is greater than 3. In that case, there are only 3 non-wild
       * characters (tld), so if NONWILDCHARS is 4, the gline will
       * be disallowed.
       * -wnder
       */
			
      nonwild = 0;
      p = user;
      while ((tmpch = *p++))
	{
	  if (!IsKWildChar(tmpch))
	    {
	      /*
	       * If we find enough non-wild characters, we can
	       * break - no point in searching further.
	       */
	      if (++nonwild >= NONWILDCHARS)
		break;
	    }
	}
			
      if (nonwild < NONWILDCHARS)
	{
	  /*
	   * The user portion did not contain enough non-wild
	   * characters, try the host.
	   */
	  p = host;
	  while ((tmpch = *p++))
	    {
	      if (!IsKWildChar(tmpch))
		if (++nonwild >= NONWILDCHARS)
		  break;
	    }
	}
	  
      if (nonwild < NONWILDCHARS)
	{
	  /*
	   * Not enough non-wild characters were found, assume
	   * they are trying to gline *@*.
	   */
	  if (MyClient(sptr))
	    sendto_one(sptr,
		       ":%s NOTICE %s :Please include at least %d non-wildcard characters with the user@host",
		       me.name,
		       parv[0],
		       NONWILDCHARS);
	  
	  return 0;
	}
			
      reason = parv[2];

      /* If at least 3 opers agree this user should be G lined then do it */
      check_majority_gline(sptr,
			   sptr->name,
			   (const char *)sptr->user,
			   sptr->host,
			   me.name,
			   user,
			   host,
			   reason);
	  
      sendto_cap_serv_butone(CAP_GLN,
			     NULL, ":%s GLINE %s %s %s :%s",
			     me.name,
			     sptr->name,
			     user,
			     host,
			     reason);
    }
  else
    {
      sendto_one(sptr,":%s NOTICE %s :GLINE disabled",me.name,parv[0]);  
    }

  return 0;
}

/*
 * ms_gline()
 *
 * inputs       - The usual for a m_ function
 * output       -
 * side effects -
 *
 * Place a G line if 3 opers agree on the identical user@host
 * 
 */
/* Allow this server to pass along GLINE if received and
 * GLINES is not defined.
 */

int     ms_gline(struct Client *cptr,
                struct Client *sptr,
                int parc,
                char *parv[])
{
  struct Client *rcptr;
  const char *oper_nick = NULL;        /* nick of oper requesting GLINE */
  const char *oper_user = NULL;        /* username of oper requesting GLINE */
  const char *oper_host = NULL;        /* hostname of oper requesting GLINE */
  const char *oper_server = NULL;      /* server of oper requesting GLINE */
  const char *user = NULL;
  const char *host = NULL;             /* user and host of GLINE "victim" */
  const char *reason = NULL;           /* reason for "victims" demise */

  if(!IsServer(sptr))
    return(0);

  /* Always good to be paranoid about arguments */
  if(parc < 5)
    return 0;

  oper_nick = parv[1];
  user = parv[2];
  host = parv[3];
  reason = parv[4];

  if ((rcptr = hash_find_client(oper_nick,(struct Client *)NULL)))
    {
      if(!IsPerson(rcptr))
	return 0;
    }
  else
    return 0;

  if ((oper_user = (const char *)rcptr->user) == NULL)
    return 0;

  if ((oper_host = rcptr->host) == NULL)
    return 0;

  if (rcptr->user && rcptr->user->server)
    oper_server = rcptr->user->server;
  else
    return 0;

  sendto_serv_butone(sptr, ":%s GLINE %s %s %s :%s",
		     sptr->name,
		     oper_nick,
		     user,
		     host,
		     reason);

  if (ConfigFileEntry.glines)
    {
      log_gline_request(oper_nick,oper_user,oper_host,oper_server,
			user,host,reason);

      sendto_realops("%s!%s@%s on %s is requesting gline for [%s@%s] [%s]",
		     oper_nick,
		     oper_user,
		     oper_host,
		     oper_server,
		     user,
		     host,
		     reason);

      /* If at least 3 opers agree this user should be G lined then do it */
      check_majority_gline(sptr,
			   oper_nick,
			   oper_user,
			   oper_host,
			   oper_server,
			   user,
			   host,
			   reason);
    }
  return 0;
}

/*
 * check_majority_gline
 *
 * inputs	- ...
 * output	- NONE
 * side effects	- if a majority agree, place the gline locally
 */
void
check_majority_gline(struct Client *sptr,
		     const char *oper_nick,
		     const char *oper_user,
		     const char *oper_host,
		     const char *oper_server,
		     const char *user,
		     const char *host,
		     const char *reason)
{
  if(majority_gline(sptr,oper_nick,oper_user, oper_host,
		    oper_server, user, host, reason))
    set_local_gline(oper_nick,oper_user,oper_host,oper_server,
		    user,host,reason);
}

/*
 * set_local_gline
 *
 * inputs	- pointer to oper nick
 * 		- pointer to oper username
 * 		- pointer to oper host
 *		- pointer to oper server
 *		- pointer to victim user
 *		- pointer to victim host
 *		- pointer reason
 * output	- NONE
 * side effects	-
 */
void set_local_gline(const char *oper_nick,
		     const char *oper_user,
		     const char *oper_host,
		     const char *oper_server,
		     const char *user,
		     const char *host,
		     const char *reason)
{
  char buffer[IRCD_BUFSIZE];
  struct ConfItem *aconf;
  const char *current_date;

  current_date = smalldate((time_t) 0);
          
  aconf = make_conf();
  aconf->status = CONF_KILL;
  DupString(aconf->host, host);

  ircsprintf(buffer, "%s (%s)",reason,current_date);
      
  DupString(aconf->passwd, buffer);
  DupString(aconf->name, (char *)user);
  DupString(aconf->host, (char *)host);
  aconf->hold = CurrentTime + ConfigFileEntry.gline_time;
  add_gline(aconf);
      
  sendto_realops("%s!%s@%s on %s has triggered gline for [%s@%s] [%s]",
		 oper_nick,
		 oper_user,
		 oper_host,
		 oper_server,
		 user,
		 host,
		 reason);
      
  check_klines();
}


/*
 * log_gline_request()
 *
 */
void
log_gline_request(
		  const char *oper_nick,
		  const char *oper_user,
		  const char *oper_host,
		  const char* oper_server,
		  const char *user,
		  const char *host,
		  const char *reason)
{
  char        buffer[1024];
  char        filenamebuf[PATH_MAX + 1];
  static char timebuffer[MAX_DATE_STRING];
  struct tm*  tmptr;
  int         out;

  ircsprintf(filenamebuf, "%s.%s", 
             ConfigFileEntry.glinefile, small_file_date((time_t)0));
  if ((out = file_open(filenamebuf, O_RDWR|O_APPEND|O_CREAT,0644))==-1)
    {
      sendto_realops("*** Problem opening %s",filenamebuf);
      return;
    }

  tmptr = localtime(&CurrentTime);
  strftime(timebuffer, MAX_DATE_STRING, "%Y/%m/%d %H:%M:%S", tmptr);

  ircsprintf(buffer,
           "#Gline for %s@%s [%s] requested by %s!%s@%s on %s at %s\n",
           user,host,reason,
           oper_nick,oper_user,oper_host,oper_server,
           timebuffer);

  if (write(out, buffer, strlen(buffer)) <= 0)
    {
      sendto_realops("*** Problem writing to %s",filenamebuf);
    }
  file_close(out);
}

/*
 * log_gline()
 *
 */
void
log_gline(struct Client *sptr,
	  GLINE_PENDING *gline_pending_ptr,
	  const char *oper_nick,
	  const char *oper_user,
	  const char *oper_host,
	  const char *oper_server,
	  const char *user,
	  const char *host,
	  const char *reason)
{
  char         buffer[1024];
  char         filenamebuf[PATH_MAX + 1];
  static  char timebuffer[MAX_DATE_STRING + 1];
  struct tm*   tmptr;
  FBFILE       *out;

  ircsprintf(filenamebuf, "%s.%s", 
                ConfigFileEntry.glinefile, small_file_date((time_t) 0));

  if ((out = fbopen(filenamebuf, "a")) == NULL)
    {
      return;
    }

  tmptr = localtime(&CurrentTime);
  strftime(timebuffer, MAX_DATE_STRING, "%Y/%m/%d %H:%M:%S", tmptr);

  ircsprintf(buffer,"#Gline for %s@%s %s added by the following\n",
                   user,host,timebuffer);

  if (safe_write(sptr, filenamebuf, out, buffer))
    {
      fbclose(out);
      return;
    }

  ircsprintf(buffer, "#%s!%s@%s on %s [%s]\n",
                   gline_pending_ptr->oper_nick1,
                   gline_pending_ptr->oper_user1,
                   gline_pending_ptr->oper_host1,
                   gline_pending_ptr->oper_server1,
                   (gline_pending_ptr->reason1)?
                   (gline_pending_ptr->reason1):"No reason");

  if (safe_write(sptr,filenamebuf,out,buffer))
    {
      fbclose(out);
      return;
    }

  ircsprintf(buffer, "#%s!%s@%s on %s [%s]\n",
                   gline_pending_ptr->oper_nick2,
                   gline_pending_ptr->oper_user2,
                   gline_pending_ptr->oper_host2,
                   gline_pending_ptr->oper_server2,
                   (gline_pending_ptr->reason2)?
                   (gline_pending_ptr->reason2):"No reason");

  if (safe_write(sptr, filenamebuf, out, buffer))
    return;

  ircsprintf(buffer, "#%s!%s@%s on %s [%s]\n",
                   oper_nick,
                   oper_user,
                   oper_host,
                   oper_server,
                   (reason)?reason:"No reason");

  if (safe_write(sptr,filenamebuf,out,buffer))
    return;

  ircsprintf(buffer, "K:%s:%s:%s\n", host,user,reason);
  if (safe_write(sptr,filenamebuf,out,buffer))
    return;

  fbclose(out);
}

/*
 * flush_glines
 * 
 * inputs       - NONE
 * output       - NONE
 * side effects -
 *
 * Get rid of all placed G lines, hopefully to be replaced by gline.log
 * placed k-lines
 */
void flush_glines()
{
  struct ConfItem *kill_list_ptr;

  if((kill_list_ptr = glines))
    {
      while(kill_list_ptr)
        {
          glines = kill_list_ptr->next;
          free_conf(kill_list_ptr);
          kill_list_ptr = glines;
        }
    }
  glines = (struct ConfItem *)NULL;
}

/* find_gkill
 *
 * inputs       - struct Client pointer to a Client struct
 * output       - struct ConfItem pointer if a gline was found for this client
 * side effects - none
 */

struct ConfItem *find_gkill(struct Client* cptr, char* username)
{
  assert(0 != cptr);
  return (IsElined(cptr)) ? 0 : find_is_glined(cptr->host, username);
}

/*
 * find_is_glined
 * inputs       - hostname
 *              - username
 * output       - pointer to struct ConfItem if user@host glined
 * side effects -
 */

struct ConfItem* find_is_glined(const char* host, const char* name)
{
  struct ConfItem *kill_list_ptr;     /* used for the link list only */
  struct ConfItem *last_list_ptr;
  struct ConfItem *tmp_list_ptr;

  /* gline handling... exactly like temporary klines 
   * I expect this list to be very tiny. (crosses fingers) so CPU
   * time in this, should be minimum.
   */

  if(glines)
    {
      kill_list_ptr = last_list_ptr = glines;

      while(kill_list_ptr)
        {
          if(kill_list_ptr->hold <= CurrentTime)  /* a gline has expired */
            {
              if(glines == kill_list_ptr)
                {
                  /* Its pointing to first one in link list*/
                  /* so, bypass this one, remember bad things can happen
                     if you try to use an already freed pointer.. */

                  glines = last_list_ptr = tmp_list_ptr =
                    kill_list_ptr->next;
                }
              else
                {
                  /* its in the middle of the list, so link around it */
                  tmp_list_ptr = last_list_ptr->next = kill_list_ptr->next;
                }

              free_conf(kill_list_ptr);
              kill_list_ptr = tmp_list_ptr;
            }
          else
            {
              if( (kill_list_ptr->name && (!name || match(kill_list_ptr->name,
                 name))) && (kill_list_ptr->host &&
                   (!host || match(kill_list_ptr->host,host))))
                return(kill_list_ptr);
              last_list_ptr = kill_list_ptr;
              kill_list_ptr = kill_list_ptr->next;
            }
        }
    }

  return((struct ConfItem *)NULL);
}

/* report_glines
 *
 * inputs       - struct Client pointer
 * output       - NONE
 * side effects - 
 *
 * report pending glines, and placed glines.
 */
void report_glines(struct Client *sptr)
{
  GLINE_PENDING *gline_pending_ptr;
  struct ConfItem *kill_list_ptr;
  struct ConfItem *last_list_ptr;
  struct ConfItem *tmp_list_ptr;
  char timebuffer[MAX_DATE_STRING];
  struct tm *tmptr;
  char *host;
  char *name;
  char *reason;

  expire_pending_glines();      /* This is not the g line list, but
                                   the pending possible g line list */

  if((gline_pending_ptr = pending_glines))
    {
      sendto_one(sptr,":%s NOTICE %s :Pending G-lines", me.name,
                         sptr->name);
      while(gline_pending_ptr)
        {
          tmptr = localtime(&gline_pending_ptr->time_request1);
          strftime(timebuffer, MAX_DATE_STRING, "%Y/%m/%d %H:%M:%S", tmptr);

          sendto_one(sptr,":%s NOTICE %s :1) %s!%s@%s on %s requested gline at %s for %s@%s [%s]",
                     me.name,sptr->name,
                     gline_pending_ptr->oper_nick1,
                     gline_pending_ptr->oper_user1,
                     gline_pending_ptr->oper_host1,
                     gline_pending_ptr->oper_server1,
                     timebuffer,
                     gline_pending_ptr->user,
                     gline_pending_ptr->host,
                     gline_pending_ptr->reason1);

          if(gline_pending_ptr->oper_nick2[0])
            {
              tmptr = localtime(&gline_pending_ptr->time_request2);
              strftime(timebuffer, MAX_DATE_STRING, "%Y/%m/%d %H:%M:%S", tmptr);
              sendto_one(sptr,
              ":%s NOTICE %s :2) %s!%s@%s on %s requested gline at %s for %s@%s [%s]",
                         me.name,sptr->name,
                         gline_pending_ptr->oper_nick2,
                         gline_pending_ptr->oper_user2,
                         gline_pending_ptr->oper_host2,
                         gline_pending_ptr->oper_server2,
                         timebuffer,
                         gline_pending_ptr->user,
                         gline_pending_ptr->host,
                         gline_pending_ptr->reason2);
            }
          gline_pending_ptr = gline_pending_ptr->next;
        }
      sendto_one(sptr,":%s NOTICE %s :End of Pending G-lines", me.name,
                 sptr->name);
    }

  if(glines)
    {
      kill_list_ptr = last_list_ptr = glines;

      while(kill_list_ptr)
        {
          if(kill_list_ptr->hold <= CurrentTime)   /* gline has expired */
            {
              if(glines == kill_list_ptr)
                {
                  glines = last_list_ptr = tmp_list_ptr =
                    kill_list_ptr->next;
                }
              else
                {
                  /* its in the middle of the list, so link around it */
                  tmp_list_ptr = last_list_ptr->next = kill_list_ptr->next;
                }

              free_conf(kill_list_ptr);
              kill_list_ptr = tmp_list_ptr;
            }
          else
            {
              if(kill_list_ptr->host)
                host = kill_list_ptr->host;
              else
                host = "*";

              if(kill_list_ptr->name)
                name = kill_list_ptr->name;
              else
                name = "*";

              if(kill_list_ptr->passwd)
                reason = kill_list_ptr->passwd;
              else
                reason = "No Reason";

              sendto_one(sptr,form_str(RPL_STATSKLINE), me.name,
                         sptr->name, 'G' , host, name, reason);

              last_list_ptr = kill_list_ptr;
              kill_list_ptr = kill_list_ptr->next;
            }
        }
    }
}

/*
 * expire_pending_glines
 * 
 * inputs       - NONE
 * output       - NONE
 * side effects -
 *
 * Go through the pending gline list, expire any that haven't had
 * enough "votes" in the time period allowed
 */

void
expire_pending_glines()
{
  GLINE_PENDING *gline_pending_ptr;
  GLINE_PENDING *last_gline_pending_ptr;
  GLINE_PENDING *tmp_pending_ptr;

  if(pending_glines == (GLINE_PENDING *)NULL)
    return;

  last_gline_pending_ptr = (GLINE_PENDING *)NULL;
  gline_pending_ptr = pending_glines;

  while(gline_pending_ptr)
    {
      if( (gline_pending_ptr->last_gline_time + GLINE_PENDING_EXPIRE) <= CurrentTime )
        {
          if(last_gline_pending_ptr)
            last_gline_pending_ptr->next = gline_pending_ptr->next;
          else
            pending_glines = gline_pending_ptr->next;

          tmp_pending_ptr = gline_pending_ptr;
          gline_pending_ptr = gline_pending_ptr->next;
          MyFree(tmp_pending_ptr->reason1);
          MyFree(tmp_pending_ptr->reason2);
          MyFree(tmp_pending_ptr);
        }
      else
        {
          last_gline_pending_ptr = gline_pending_ptr;
          gline_pending_ptr = gline_pending_ptr->next;
        }
    }
}

void
add_new_majority_gline(const char* oper_nick,
		       const char* oper_user,
		       const char* oper_host,
		       const char* oper_server,
		       const char* user,
		       const char* host,
		       const char* reason)
{
  GLINE_PENDING* pending = (GLINE_PENDING*) MyMalloc(sizeof(GLINE_PENDING));
  assert(0 != pending);

  memset(pending, 0, sizeof(GLINE_PENDING));

  strncpy_irc(pending->oper_nick1, oper_nick, NICKLEN);
  /* pending->oper_nick2[0] = '\0'; */

  strncpy_irc(pending->oper_user1, oper_user, USERLEN);
  /* pending->oper_user2[0] = '\0'; */

  strncpy_irc(pending->oper_host1, oper_host, HOSTLEN);
   /* pending->oper_host2[0] = '\0'; */

  pending->oper_server1 = find_or_add(oper_server);

  strncpy_irc(pending->user, user, USERLEN);
  strncpy_irc(pending->host, host, HOSTLEN);
  DupString(pending->reason1, reason);
  pending->reason2 = NULL;

  pending->last_gline_time = CurrentTime;
  pending->time_request1 = CurrentTime;

  pending->next = pending_glines;
  pending_glines = pending;
}

/*
 * majority_gline()
 *
 * inputs       - oper_nick, oper_user, oper_host, oper_server
 *                user,host reason
 *
 * output       - YES if there are 3 different opers on 3 different servers
 *                agreeing to this gline, NO if there are not.
 * Side effects -
 *      See if there is a majority agreement on a GLINE on the given user
 *      There must be at least 3 different opers agreeing on this GLINE
 *
 *      Expire old entries.
 */
int
majority_gline(struct Client *sptr,
	       const char *oper_nick,
	       const char *oper_user,
	       const char *oper_host,
	       const char* oper_server,
	       const char *user,
	       const char *host,
	       const char *reason)
{
  GLINE_PENDING* gline_pending_ptr;

  /* special case condition where there are no pending glines */

  if (pending_glines == NULL) /* first gline request placed */
    {
      add_new_majority_gline(oper_nick, oper_user, oper_host, oper_server,
                             user, host, reason);
      return NO;
    }

  expire_pending_glines();

  for (gline_pending_ptr = pending_glines;
      gline_pending_ptr; gline_pending_ptr = gline_pending_ptr->next)
    {
      if( (irccmp(gline_pending_ptr->user,user) == 0) &&
          (irccmp(gline_pending_ptr->host,host) ==0 ) )
        {
          if(((irccmp(gline_pending_ptr->oper_user1,oper_user) == 0) &&
              (irccmp(gline_pending_ptr->oper_host1,oper_host) == 0)) ||
              (irccmp(gline_pending_ptr->oper_server1,oper_server) == 0) )
            {
              /* This oper or server has already "voted" */
              sendto_realops("oper or server has already voted");
              return NO;
            }

          if(gline_pending_ptr->oper_user2[0] != '\0')
            {
              /* if two other opers on two different servers have voted yes */

              if(((irccmp(gline_pending_ptr->oper_user2,oper_user)==0) &&
                  (irccmp(gline_pending_ptr->oper_host2,oper_host)==0)) ||
                  (irccmp(gline_pending_ptr->oper_server2,oper_server)==0))
                {
                  /* This oper or server has already "voted" */
                  sendto_realops("oper or server has already voted");
                  return NO;
                }

              if(find_is_glined(host, user))
                return NO;

              if(find_is_klined(host, user, 0))
                return NO;

              log_gline(sptr,gline_pending_ptr,
                        oper_nick,oper_user,oper_host,oper_server,
                        user,host,reason);
              return YES;
            }
          else
            {
              strncpy_irc(gline_pending_ptr->oper_nick2, oper_nick, NICKLEN);
              strncpy_irc(gline_pending_ptr->oper_user2, oper_user, USERLEN);
              strncpy_irc(gline_pending_ptr->oper_host2, oper_host, HOSTLEN);
              DupString(gline_pending_ptr->reason2, reason);
              gline_pending_ptr->oper_server2 = find_or_add(oper_server);
              gline_pending_ptr->last_gline_time = CurrentTime;
              gline_pending_ptr->time_request1 = CurrentTime;
              return NO;
            }
        }
    }
  /* Didn't find this user@host gline in pending gline list
   * so add it.
   */
  add_new_majority_gline(oper_nick, oper_user, oper_host, oper_server,
                         user, host, reason);
  return NO;
}

/* add_gline
 *
 * inputs       - pointer to struct ConfItem
 * output       - none
 * Side effects - links in given struct ConfItem into gline link list
 */

void
add_gline(struct ConfItem *aconf)
{
  aconf->next = glines;
  glines = aconf;
}

/*
 * remove_gline_match
 *
 * inputs       - user@host
 * output       - 1 if successfully removed, otherwise 0
 * side effects -
 */
int
remove_gline_match(const char* user, const char* host)
{
  struct ConfItem *kill_list_ptr;     /* used for the link list only */
  struct ConfItem *last_list_ptr;
  struct ConfItem *tmp_list_ptr;

  if(glines)
    {
      kill_list_ptr = last_list_ptr = glines;

      while(kill_list_ptr)
        {
          if(!irccmp(kill_list_ptr->host,host) &&
             !irccmp(kill_list_ptr->name,user))  /* this gline matches */
            {
              if(glines == kill_list_ptr)
                {
                  /* Its pointing to first one in link list*/
                  /* so, bypass this one, remember bad things can happen
                     if you try to use an already freed pointer.. */

                  glines = last_list_ptr = tmp_list_ptr =
                    kill_list_ptr->next;
                }
              else
                {
                  /* its in the middle of the list, so link around it */
                  tmp_list_ptr = last_list_ptr->next = kill_list_ptr->next;
                }
              free_conf(kill_list_ptr);
              kill_list_ptr = tmp_list_ptr;
              return 1;
            }
          else
            {
              last_list_ptr = kill_list_ptr;
              kill_list_ptr = kill_list_ptr->next;
            }
        }
      return 0;
    }
  else
    return 0;
}

