/************************************************************************
 *   IRC - Internet Relay Chat, src/m_list.c
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
 *   $Id: m_list.c,v 7.1 1999/08/20 04:38:24 tomh Exp $
 */
#include "m_commands.h"
#include "channel.h"
#include "client.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"

#include <string.h>
/*
 * m_functions execute protocol messages on this server:
 *
 *      cptr    is always NON-NULL, pointing to a *LOCAL* client
 *              structure (with an open socket connected!). This
 *              identifies the physical socket where the message
 *              originated (or which caused the m_function to be
 *              executed--some m_functions may call others...).
 *
 *      sptr    is the source of the message, defined by the
 *              prefix part of the message if present. If not
 *              or prefix not found, then sptr==cptr.
 *
 *              (!IsServer(cptr)) => (cptr == sptr), because
 *              prefixes are taken *only* from servers...
 *
 *              (IsServer(cptr))
 *                      (sptr == cptr) => the message didn't
 *                      have the prefix.
 *
 *                      (sptr != cptr && IsServer(sptr) means
 *                      the prefix specified servername. (?)
 *
 *                      (sptr != cptr && !IsServer(sptr) means
 *                      that message originated from a remote
 *                      user (not local).
 *
 *              combining
 *
 *              (!IsServer(sptr)) means that, sptr can safely
 *              taken as defining the target structure of the
 *              message in this server.
 *
 *      *Always* true (if 'parse' and others are working correct):
 *
 *      1)      sptr->from == cptr  (note: cptr->from == cptr)
 *
 *      2)      MyConnect(sptr) <=> sptr == cptr (e.g. sptr
 *              *cannot* be a local connection, unless it's
 *              actually cptr!). [MyConnect(x) should probably
 *              be defined as (x == x->from) --msa ]
 *
 *      parc    number of variable parameter strings (if zero,
 *              parv is allowed to be NULL)
 *
 *      parv    a NULL terminated list of parameter pointers,
 *
 *                      parv[0], sender (prefix string), if not present
 *                              this points to an empty string.
 *                      parv[1]...parv[parc-1]
 *                              pointers to additional parameters
 *                      parv[parc] == NULL, *always*
 *
 *              note:   it is guaranteed that parv[0]..parv[parc-1] are all
 *                      non-NULL pointers.
 */


/*
** m_list
**      parv[0] = sender prefix
**      parv[1] = channel
*/
int m_list(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  struct Channel* chptr;
  char*           name;
  char*           p = NULL;
  /* anti flooding code,
   * I did have this in parse.c with a table lookup
   * but I think this will be less inefficient doing it in each
   * function that absolutely needs it
   *
   * -Dianora
   */
  static time_t last_used=0L;
  int i,j;

  /* throw away non local list requests that do get here -Dianora */
  if(!MyConnect(sptr))
    return 0;

  if(!IsAnOper(sptr))
    {
      if(((last_used + PACE_WAIT) > CurrentTime) && (!IsDoingList(sptr)))
        return 0;
      else
        last_used = CurrentTime;
    }

  /* right.. if we are already involved in a "blocked" /list, we will simply
     continue where we left off */
  if (IsDoingList(sptr)) {
    if (sptr->listprogress != -1) {
      for (i=sptr->listprogress; i<CH_MAX; i++) {
        int progress2 = sptr->listprogress2;
        for (j=0, chptr=(struct Channel*)(hash_get_channel_block(i).list);
             (chptr) && (j<hash_get_channel_block(i).links); chptr=chptr->hnextch, j++) {
          if (j<progress2) continue;  /* wind up to listprogress2 */
          if (!sptr->user ||
              (SecretChannel(chptr) && !IsMember(sptr, chptr)))
            continue;
          sendto_one(sptr, form_str(RPL_LIST), me.name, parv[0],
                     ShowChannel(sptr, chptr)?chptr->chname:"*",
                     chptr->users,
                     ShowChannel(sptr, chptr)?chptr->topic:"");
          if (IsSendqPopped(sptr)) {
            /* we popped again! : P */
            sptr->listprogress=i;
            sptr->listprogress2=j;
            return 0;
          }
        }
        sptr->listprogress2 = 0;
      }
    }
    sendto_one(sptr, form_str(RPL_LISTEND), me.name, parv[0]);
    if (IsSendqPopped(sptr)) { /* popped with the RPL_LISTEND code. d0h */
      sptr->listprogress = -1;
      return 0;
    }
    ClearDoingList(sptr);   /* yupo, its over */
    return 0;
    
  }
  
  sendto_one(sptr, form_str(RPL_LISTSTART), me.name, parv[0]);

  if (parc < 2 || BadPtr(parv[1]))
    {
      SetDoingList(sptr);     /* only set if its a full list */
      ClearSendqPop(sptr);    /* just to make sure */
      /* we'll do this by looking through each hash table bucket */
      for (i=0; i<CH_MAX; i++) {
        for (j=0, chptr = (struct Channel*)(hash_get_channel_block(i).list);
             (chptr) && (j<hash_get_channel_block(i).links); chptr = chptr->hnextch, j++) {
          if (!sptr->user ||
              (SecretChannel(chptr) && !IsMember(sptr, chptr)))
            continue;
          /* EVIL!  sendto_one doesnt return status of any kind!  Forcing us
             to make up yet another stupid client flag (we could just
             negate the DOING_LIST flag, but that might confuse people) -good*/
          sendto_one(sptr, form_str(RPL_LIST), me.name, parv[0],
                     ShowChannel(sptr, chptr)?chptr->chname:"*",
                     chptr->users,
                     ShowChannel(sptr, chptr)?chptr->topic:"");
          if (IsSendqPopped(sptr)) {
            /* GAAH!  We popped our sendq.  Mark our location in the /list */
            sptr->listprogress=i;
            sptr->listprogress2=j;
            return 0;
          }
        }
      
      }

      sendto_one(sptr, form_str(RPL_LISTEND), me.name, parv[0]);
      if (IsSendqPopped(sptr)) {
        sptr->listprogress=-1;
        return 0;
      }
      ClearDoingList(sptr);   /* yupo, its over */
      return 0;
    }   
          

  /* Don't route list, no need for it - Dianora */
  /*
    if (hunt_server(cptr, sptr, ":%s LIST %s %s", 2, parc, parv))
      return 0;
      */

  p = strchr(parv[1],',');
  if(p)
    *p = '\0';
  name = parv[1]; /* strtoken(&p, parv[1], ","); */

  /* while(name) */
  if(name)
    {
      chptr = hash_find_channel(name, NullChn);
      if (chptr && ShowChannel(sptr, chptr) && sptr->user)
        sendto_one(sptr, form_str(RPL_LIST), me.name, parv[0],
                   ShowChannel(sptr,chptr) ? name : "*",
                   chptr->users, chptr->topic);
      /*      name = strtoken(&p, (char *)NULL, ","); */
    }
  sendto_one(sptr, form_str(RPL_LISTEND), me.name, parv[0]);
  return 0;
}

