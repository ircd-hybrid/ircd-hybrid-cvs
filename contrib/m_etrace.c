/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_etrace.c: Traces a path to a client/server.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: m_etrace.c,v 1.3 2005/07/16 12:19:38 michael Exp $
 */

#include "stdinc.h"
#include "handlers.h"
#include "tools.h"
#include "hook.h"
#include "client.h"
#include "hash.h"
#include "common.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "fdlist.h"
#include "s_bsd.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "irc_getnameinfo.h"

#define FORM_STR_RPL_ETRACE	":%s 709 %s %s %s %s %s %s :%s"

static void mo_etrace(struct Client *, struct Client *, int, char *[]);
static void etrace_spy(struct Client *);

struct Message etrace_msgtab = {
  "ETRACE", 0, 0, 0, 0, MFLG_SLOW, 0,
  {m_unregistered, m_ignore, m_ignore, m_ignore, mo_etrace, m_ignore}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  hook_add_event("doing_etrace");
  mod_add_cmd(&etrace_msgtab);
}

void
_moddeinit(void)
{
  hook_del_event("doing_etrace");
  mod_del_cmd(&etrace_msgtab);
}
const char *_version = "$Revision: 1.3 $";
#endif

static void report_this_status(struct Client *, struct Client *);

/* mo_etrace()
 *      parv[0] = sender prefix
 *      parv[1] = servername
 */
static void
mo_etrace(struct Client *client_p, struct Client *source_p,
	  int parc, char *parv[])
{
  const char *tname = NULL;
  struct Client *target_p = NULL;
  int wilds = 0;
  int do_all = 0;
  dlink_node *ptr;

  etrace_spy(source_p);

  if (parc > 0)
  {
    tname = parv[1];
    if (tname != NULL)
      wilds = strchr(tname, '*') || strchr(tname, '?');
    else
      tname = "*";
  }
  else
  {
    do_all = 1;
    tname = "*";
  }

  set_time();

  if (!wilds && !do_all)
  {
    target_p = find_client(tname);

    if (target_p && MyClient(target_p))
      report_this_status(source_p, target_p);
      
    sendto_one(source_p, form_str(RPL_ENDOFTRACE), me.name, 
	       source_p->name, tname);
    return;
  }

  DLINK_FOREACH(ptr, local_client_list.head)
  {
    target_p = ptr->data;

    if (wilds)
    {
      if (match(tname, target_p->name) || match(target_p->name, tname))
	report_this_status(source_p, target_p);
    }
    else
    {
      report_this_status(source_p, target_p);
    }
  }

  sendto_one(source_p, form_str(RPL_ENDOFTRACE), me.name,
	     source_p->name, tname);
}

/* report_this_status()
 *
 * inputs	- pointer to client to report to
 * 		- pointer to client to report about
 * output	- NONE
 * side effects - NONE
 */
static void
report_this_status(struct Client *source_p, struct Client *target_p)
{
  const char *name;
  const char *class_name;
  char ip[HOSTIPLEN];

  /* Should this be sockhost? - stu */
  irc_getnameinfo((struct sockaddr*)&target_p->localClient->ip, 
        target_p->localClient->ip.ss_len, ip, HOSTIPLEN, NULL, 0, 
        NI_NUMERICHOST);

  name = get_client_name(target_p, HIDE_IP);
  class_name = get_client_class(target_p);

  set_time();

  if (target_p->status == STAT_CLIENT)
  {
    if (ConfigFileEntry.hide_spoof_ips)
      sendto_one(source_p, FORM_STR_RPL_ETRACE,
		 me.name, source_p->name,
		 IsOper(target_p) ? "Oper" : "User",
		 class_name,
		 target_p->name, target_p->username,
		 IsIPSpoof(target_p) ? "255.255.255.255" : ip,
		 target_p->info);
    else
      sendto_one(source_p, FORM_STR_RPL_ETRACE,
		 me.name, source_p->name, 
		 IsOper(target_p) ? "Oper" : "User", 
		 class_name,
		 target_p->name, target_p->username, ip,
		 target_p->info);
  }
}

/* etrace_spy()
 *
 * input        - pointer to client
 * output       - none
 * side effects - hook event doing_etrace is called
 */
static void
etrace_spy(struct Client *source_p)
{
  struct hook_spy_data data;

  data.source_p = source_p;

  hook_call_event("doing_etrace", &data);
}
