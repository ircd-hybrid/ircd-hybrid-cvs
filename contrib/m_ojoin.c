/*   contrib/m_ojoin.c
 *   Copyright (C) 2002, 2003, 2004, 2005 Hybrid Development Team
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
 *   $Id: m_ojoin.c,v 1.32 2005/06/23 10:03:59 michael Exp $
 */

#include "stdinc.h"
#include "tools.h"
#include "handlers.h"
#include "channel.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "irc_string.h"
#include "hash.h"
#include "msg.h"
#include "s_serv.h"
#include "modules.h"
#include "list.h"
#include "channel_mode.h"

static void mo_ojoin(struct Client *, struct Client *, int, char *[]);

struct Message ojoin_msgtab = {
  "OJOIN", 0, 0, 2, 0, MFLG_SLOW, 0,
  { m_unregistered, m_not_oper, m_ignore, m_ignore, mo_ojoin, m_ignore }
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&ojoin_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&ojoin_msgtab);
}

const char *_version = "$Revision: 1.32 $";
#endif

/* mo_ojoin()
 *      parv[0] = sender prefix
 *      parv[1] = channels separated by commas
 */
static void
mo_ojoin(struct Client *client_p, struct Client *source_p,
         int parc, char *parv[])
{
  struct Channel *chptr = NULL;
  char modeletter = '\0';
  char *name = parv[1];
  char *t = NULL;
  int move_me = 1;
  unsigned int tmp_flags;

  /* admins only */
  if (!IsAdmin(source_p))
  {
    sendto_one(source_p, form_str(ERR_NOPRIVILEGES),
               me.name, source_p->name);
    return;
  }

  for (name = strtoken(&t, name, ","); name;
       name = strtoken(&t, NULL, ","))
  {
    move_me = 1;

    switch (*name)
    {
      case '@':
        tmp_flags = CHFL_CHANOP;
        modeletter = 'o';
        ++name;
        break;
#ifdef HALFOPS
      case '%':
        tmp_flags = CHFL_HALFOP;
        modeletter = 'h';
        ++name;
        break;
#endif
      case '+':
        tmp_flags = CHFL_VOICE;
        modeletter = 'v';
        ++name;
        break;
      case '#':
      case '&':
        tmp_flags = 0;
	modeletter = '\0';
	break;

      default:
        sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL),
                   me.name, source_p->name, name);
	continue;
    }

    /* Error checking here */
    if ((chptr = hash_find_channel(name)) == NULL)
    {
      sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL),
                 me.name, source_p->name, name);
    }
    else if (IsMember(source_p, chptr))
    {
      sendto_one(source_p, ":%s NOTICE %s :Please part %s before using OJOIN",
                 me.name, source_p->name, name);
    }
    else
    {
      if (move_me == 1)
        --name;

      add_user_to_channel(chptr, source_p, tmp_flags);

      if (chptr->chname[0] == '#')
      {
        sendto_server(client_p, source_p, chptr, CAP_TS6, NOCAPS, LL_ICLIENT,
                      ":%s SJOIN %lu %s + :%c%s",
                      me.id, (unsigned long)chptr->channelts, chptr->chname,
                      (modeletter != '\0') ? *name : ' ',
                      source_p->id);
        sendto_server(client_p, source_p, chptr, NOCAPS, CAP_TS6, LL_ICLIENT, 
                      ":%s SJOIN %lu %s + :%c%s", me.name,
                      (unsigned long)chptr->channelts,
                      chptr->chname, (modeletter != '\0') ? *name : ' ',
                      source_p->name);
      }

      sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s JOIN %s",
                           source_p->name, source_p->username,
                           source_p->host,
                           chptr->chname);

      if (modeletter != '\0')
        sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s +%c %s",
                             me.name, chptr->chname, modeletter, source_p->name);

      /* send the topic... */
      if (chptr->topic != NULL)
      {
        sendto_one(source_p, form_str(RPL_TOPIC),
                   me.name, source_p->name, chptr->chname,
                   chptr->topic);
        sendto_one(source_p, form_str(RPL_TOPICWHOTIME),
                   me.name, source_p->name, chptr->chname,
                   chptr->topic_info, chptr->topic_time);
      }

      source_p->localClient->last_join_time = CurrentTime;
      channel_member_names(source_p, chptr, 1);
    }
  }
}
