/************************************************************************
 *   IRC - Internet Relay Chat, modules/m_whois.c
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
 *   $Id: m_whois.c,v 1.58 2001/03/06 02:22:45 androsyn Exp $
 */

#include <string.h>
#include <time.h>

#include "tools.h"
#include "common.h"   /* bleah */
#include "handlers.h"
#include "client.h"
#include "common.h"   /* bleah */
#include "channel.h"
#include "vchannel.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "list.h"
#include "irc_string.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hook.h"

static int do_whois(struct Client *client_p, struct Client *source_p,
                    int parc, char *parv[]);
static int single_whois(struct Client *source_p, struct Client *aclient_p,
                        int wilds, int glob);
static void whois_person(struct Client *source_p,struct Client *aclient_p,int glob);
static int global_whois(struct Client *source_p, char *nick, int wilds, int glob);

static void m_whois(struct Client*, struct Client*, int, char**);
static void ms_whois(struct Client*, struct Client*, int, char**);
static void mo_whois(struct Client*, struct Client*, int, char**);

struct Message whois_msgtab = {
  "WHOIS", 0, 0, 0, MFLG_SLOW, 0L,
  {m_unregistered, m_whois, ms_whois, mo_whois}
};

void
_modinit(void)
{
	hook_add_event("doing_whois");
	mod_add_cmd(&whois_msgtab);
}

void
_moddeinit(void)
{
	hook_del_event("doing_whois");
  mod_del_cmd(&whois_msgtab);
}

char *_version = "20010105";

/*
** m_whois
**      parv[0] = sender prefix
**      parv[1] = nickname masklist
*/
static void m_whois(struct Client *client_p,
                   struct Client *source_p,
                   int parc,
                   char *parv[])
{
   static time_t last_used = 0;
  
  if (parc < 2)
    {
      sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN),
                 me.name, parv[0]);
      return;
    }

  if(parc > 2)
    {
      /* seeing as this is going across servers, we should limit it */
      if((last_used + ConfigFileEntry.whois_wait) > CurrentTime)
        {             
          if(MyClient(source_p))
            sendto_one(source_p,form_str(RPL_LOAD2HI),me.name,source_p->name);
          return;
        }
      else
        last_used = CurrentTime;

      if (hunt_server(client_p,source_p,":%s WHOIS %s :%s", 1, parc, parv) !=
          HUNTED_ISME)
        {
          return;
        }
      parv[1] = parv[2];

    }
  do_whois(client_p,source_p,parc,parv);
}

/*
** mo_whois
**      parv[0] = sender prefix
**      parv[1] = nickname masklist
*/
static void mo_whois(struct Client *client_p,
                    struct Client *source_p,
                    int parc,
                    char *parv[])
{
  if(parc < 2)
    {
      sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN),
                 me.name, parv[0]);
      return;
    }

  if(parc > 2)
    {
      if (hunt_server(client_p,source_p,":%s WHOIS %s :%s", 1, parc, parv) !=
          HUNTED_ISME)
        {
          return;
        }
      parv[1] = parv[2];
    }

  do_whois(client_p,source_p,parc,parv);
}


/* do_whois
 *
 * inputs	- pointer to 
 * output	- 
 * side effects -
 */
static int do_whois(struct Client *client_p, struct Client *source_p,
                    int parc, char *parv[])
{
  struct Client *aclient_p;
  char  *nick;
  char  *p = NULL;
  int   found=NO;
  int   wilds;
  int   glob=0;
  
  /* This lets us make all "whois nick" queries look the same, and all
   * "whois nick nick" queries look the same.  We have to pass it all
   * the way down to whois_person() though -- fl */

  if(parc > 2)
    glob = 1;

  nick = parv[1];
  if ( (p = strchr(parv[1],',')) )
    *p = '\0';

  (void)collapse(nick);
  wilds = (strchr(nick, '?') || strchr(nick, '*'));

  if(!wilds)
    {
      if( (aclient_p = hash_find_client(nick,(struct Client *)NULL)) )
	{
	  if (IsServer(client_p))
	    client_burst_if_needed(client_p,aclient_p);

	  if(IsPerson(aclient_p))
	    {
	      (void)single_whois(source_p,aclient_p,wilds,glob);
	      found = YES;
            }
        }
      else
	{
	  if (!ServerInfo.hub && uplink && IsCapable(uplink,CAP_LL))
	    {
	      if(glob == 1)
   	        sendto_one(uplink,":%s WHOIS %s :%s",
		  	   source_p->name, nick, nick);
	      else
		sendto_one(uplink,":%s WHOIS %s",
			   source_p->name, nick);
	      return 0;
	    }
	}
    }
  else
    {
      /* disallow wild card whois on lazylink leafs for now */

      if (!ServerInfo.hub && uplink && IsCapable(uplink,CAP_LL))
	{
	  return 0;
	}
      /* Oh-oh wilds is true so have to do it the hard expensive way */
      found = global_whois(source_p,nick,wilds,glob);
    }

  if(found)
    sendto_one(source_p, form_str(RPL_ENDOFWHOIS), me.name, parv[0], parv[1]);
  else
    sendto_one(source_p, form_str(ERR_NOSUCHNICK), me.name, parv[0], nick);

  return 0;
}

/*
 * global_whois()
 *
 * Inputs	- source_p client to report to
 *		- aclient_p client to report on
 *		- wilds whether wildchar char or not
 * Output	- if found return 1
 * Side Effects	- do a single whois on given client
 * 		  writing results to source_p
 */
static int global_whois(struct Client *source_p, char *nick,
                        int wilds, int glob)
{
  struct Client *aclient_p;
  int found = NO;

  for (aclient_p = GlobalClientList; (aclient_p = next_client(aclient_p, nick));
       aclient_p = aclient_p->next)
    {
      if (IsServer(aclient_p))
	continue;
      /*
       * I'm always last :-) and aclient_p->next == NULL!!
       */
      if (IsMe(aclient_p))
	break;
      /*
       * 'Rules' established for sending a WHOIS reply:
       *
       *
       * - if wildcards are being used dont send a reply if
       *   the querier isnt any common channels and the
       *   client in question is invisible and wildcards are
       *   in use (allow exact matches only);
       *
       * - only send replies about common or public channels
       *   the target user(s) are on;
       */

      if(!IsRegistered(aclient_p))
	continue;

      if(single_whois(source_p, aclient_p, wilds, glob))
	found = 1;
    }

  return (found);
}

/*
 * single_whois()
 *
 * Inputs	- source_p client to report to
 *		- aclient_p client to report on
 *		- wilds whether wildchar char or not
 * Output	- if found return 1
 * Side Effects	- do a single whois on given client
 * 		  writing results to source_p
 */
static int single_whois(struct Client *source_p,struct Client *aclient_p,
                        int wilds, int glob)
{
  dlink_node *ptr;
  struct Channel *chptr;
  char *name;
  int invis;
  int member;
  int showperson;
  
  if (aclient_p->name[0] == '\0')
    name = "?";
  else
    name = aclient_p->name;

  if( aclient_p->user == NULL )
    {
      sendto_one(source_p, form_str(RPL_WHOISUSER), me.name,
		 source_p->name, name,
		 aclient_p->username, aclient_p->host, aclient_p->info);
	  sendto_one(source_p, form_str(RPL_WHOISSERVER),
		 me.name, source_p->name, name, "<Unknown>",
		 "*Not On This Net*");
      return 0;
    }

  invis = IsInvisible(aclient_p);
  member = (aclient_p->user->channel.head) ? 1 : 0;
  showperson = (wilds && !invis && !member) || !wilds;

  for (ptr = aclient_p->user->channel.head; ptr; ptr = ptr->next)
    {
      chptr = ptr->data;
      member = IsMember(source_p, chptr);
      if (invis && !member)
	continue;
      if (member || (!invis && PubChannel(chptr)))
	{
	  showperson = 1;
	  break;
	}
      if (!invis && HiddenChannel(chptr) && !SecretChannel(chptr))
	{
	  showperson = 1;
	  break;
	}
    }

  if(showperson)
    whois_person(source_p,aclient_p,glob);
  return 0;
}

/*
 * whois_person()
 *
 * Inputs	- source_p client to report to
 *		- aclient_p client to report on
 * Output	- NONE
 * Side Effects	- 
 */
static void whois_person(struct Client *source_p,struct Client *aclient_p, int glob)
{
  char buf[BUFSIZE];
  char *chname;
  char *server_name;
  dlink_node  *lp;
  struct Client *a2client_p;
  struct Channel *chptr;
  struct Channel *bchan;
  int cur_len = 0;
  int mlen;
  char *t;
  int tlen;
  int reply_to_send = NO;
  struct hook_mfunc_data hd;
  
  a2client_p = find_server(aclient_p->user->server);
          
  sendto_one(source_p, form_str(RPL_WHOISUSER), me.name,
	 source_p->name, aclient_p->name,
	 aclient_p->username, aclient_p->host, aclient_p->info);
  server_name = (char *)aclient_p->user->server;

  ircsprintf(buf, form_str(RPL_WHOISCHANNELS),
	       me.name, source_p->name, aclient_p->name, "");

  mlen = strlen(buf);
  cur_len = mlen;
  t = buf + mlen;

  for (lp = aclient_p->user->channel.head; lp; lp = lp->next)
    {
      chptr = lp->data;
      chname = chptr->chname;

      if (IsVchan(chptr))
	{
	  bchan = find_bchan (chptr);
	  if (bchan != NULL)
	    chname = bchan->chname;
	}

      if (ShowChannel(source_p, chptr))
	{
	  if (chptr->mode.mode & MODE_HIDEOPS && !is_any_op(chptr,source_p))
	    {
	      ircsprintf(t,"%s ",chname);
	    }
	  else
	    {
	      ircsprintf(t,"%s%s ", channel_chanop_or_voice(chptr,aclient_p),
			 chname);
	    }

	  tlen = strlen(t);
	  t += tlen;
	  cur_len += tlen;
	  reply_to_send = YES;

	  if ((cur_len + NICKLEN) > (BUFSIZE - 4))
	    {
	      sendto_one(source_p, "%s", buf);
	      cur_len = mlen;
	      t = buf + mlen;
	      reply_to_send = NO;
	    }
	}
    }

  if (reply_to_send)
    sendto_one(source_p, "%s", buf);
          
  if ((IsOper(source_p) || !GlobalSetOptions.hide_server) || aclient_p == source_p)
    sendto_one(source_p, form_str(RPL_WHOISSERVER),
	       me.name, source_p->name, aclient_p->name, server_name,
	       a2client_p?a2client_p->info:"*Not On This Net*");
  else
    sendto_one(source_p, form_str(RPL_WHOISSERVER),
	       me.name, source_p->name, aclient_p->name,
               ServerInfo.network_name,
	       ServerInfo.network_desc);

  if (aclient_p->user->away)
    sendto_one(source_p, form_str(RPL_AWAY), me.name,
	       source_p->name, aclient_p->name, aclient_p->user->away);

  if (IsOper(aclient_p))
    {
      sendto_one(source_p, form_str(RPL_WHOISOPERATOR),
		 me.name, source_p->name, aclient_p->name);

      if (IsAdmin(aclient_p))
	sendto_one(source_p, form_str(RPL_WHOISADMIN),
		   me.name, source_p->name, aclient_p->name);
    }

  if ( (glob == 1) ||
       (MyConnect(aclient_p) && (IsOper(source_p) || !GlobalSetOptions.hide_server)) ||
       (aclient_p == source_p) )
    {
      sendto_one(source_p, form_str(RPL_WHOISIDLE),
                 me.name, source_p->name, aclient_p->name,
                 CurrentTime - aclient_p->user->last,
                 aclient_p->firsttime);
    }

  hd.client_p = aclient_p;
  hd.source_p = source_p;
  /* although we should fill in parc and parv, we don't ..
	 be careful of this when writing whois hooks */
  hook_call_event("doing_whois", &hd);
  
  return;
}

/*
** ms_whois
**      parv[0] = sender prefix
**      parv[1] = nickname masklist
*/
static void ms_whois(struct Client *client_p,
                    struct Client *source_p,
                    int parc,
                    char *parv[])
{
  struct Client *aclient_p;
  if (parc < 2)
    {
      sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN),
                 me.name, parv[0]);
      return;
    }

  /* We need this to keep compatibility with hyb6 */
  if ((parc > 2) && (aclient_p = hash_find_client(parv[1], (struct Client*)NULL)) &&
      !MyConnect(aclient_p) && IsClient(aclient_p))
    {
      client_burst_if_needed(aclient_p->from, source_p);
      sendto_one(aclient_p->from, ":%s WHOIS %s :%s", parv[0], parv[1],
                 parv[1]);  
      return;
    }

  do_whois(client_p,source_p,parc,parv);
}
