/************************************************************************
 *   IRC - Internet Relay Chat, src/m_error.c
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
 *   $Id: m_error.c,v 7.21 2001/12/30 04:19:45 db Exp $
 */
#include "handlers.h"
#include "client.h"
#include "common.h"   /* FALSE */
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "s_debug.h"
#include "msg.h"
#include "memory.h"
#include "debug.h"


struct Message error_msgtab = {
 "ERROR", 0, 0, 1, 0, MFLG_SLOW | MFLG_UNREG, 0,
  {m_error, m_ignore, ms_error, m_ignore}
};


/*
 * Note: At least at protocol level ERROR has only one parameter,
 * although this is called internally from other functions
 * --msa
 *
 *      parv[0] = sender prefix
 *      parv[*] = parameters
 */
void m_error(struct Client *client_p, struct Client *source_p,
             int parc, char *parv[])
{
  char* para;

  para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";
  
  deprintf("error", "Received ERROR message from %s: %s",
	   source_p->name, para);

  if (client_p == source_p)
    {
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
            "ERROR :from %s -- %s",
	    get_client_name(client_p, HIDE_IP), para);
      sendto_realops_flags(FLAGS_ALL, L_OPER,
            "ERROR :from %s -- %s",
	    get_client_name(client_p, MASK_IP), para);
    }
  else
    {
      sendto_realops_flags(FLAGS_ALL, L_OPER,
            "ERROR :from %s via %s -- %s",
	    source_p->name, get_client_name(client_p, MASK_IP), para);
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,"ERROR :from %s via %s -- %s",
			   source_p->name,
			   get_client_name(client_p, HIDE_IP), para);
    }
  exit_client(client_p, source_p, source_p, "ERROR");
}

void ms_error(struct Client *client_p, struct Client *source_p,
              int parc, char *parv[])
{
  char* para;

  para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";
  
  deprintf("error", "Received ERROR message from %s: %s",
	   source_p->name, para);

  if (client_p == source_p)
    sendto_realops_flags(FLAGS_ALL, L_ALL,"ERROR :from %s -- %s",
			 get_client_name(client_p, MASK_IP), para);
  else
    sendto_realops_flags(FLAGS_ALL, L_ALL,"ERROR :from %s via %s -- %s", source_p->name,
			 get_client_name(client_p, MASK_IP), para);
}
