/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  whowas.h: Header for the whowas functions.
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
 *  $Id: whowas.h,v 7.14 2003/06/26 12:19:46 michael Exp $
 */
#ifndef INCLUDED_whowas_h
#define INCLUDED_whowas_h

#include "ircd_defs.h"
#include "client.h"
#include "setup.h"

/*
 * Whowas hash table size
 *
 */
#define WW_MAX 65536

struct User;
struct Client;

/*
  lets speed this up...
  also removed away information. *tough*
  - Dianora
 */
struct Whowas
{
  int hashv;
  char name[NICKLEN];
  char username[USERLEN + 1]; 
  char hostname[HOSTLEN + 1];
  char realname[REALLEN + 1];
  char servername[HOSTLEN + 1];
  time_t logoff;
  struct Client *online; /* Pointer to new nickname for chasing or NULL */
  struct Whowas *next;  /* for hash table... */
  struct Whowas *prev;  /* for hash table... */
  struct Whowas *cnext; /* for client struct linked list */
  struct Whowas *cprev; /* for client struct linked list */
};

/*
** initwhowas
*/
extern void init_whowas(void);

/*
** add_history
**      Add the currently defined name of the client to history.
**      usually called before changing to a new name (nick).
**      Client must be a fully registered user (specifically,
**      the user structure must have been allocated).
*/
extern void add_history (struct Client *, int);

/*
** off_history
**      This must be called when the client structure is about to
**      be released. History mechanism keeps pointers to client
**      structures and it must know when they cease to exist. This
**      also implicitly calls AddHistory.
*/
extern void off_history(struct Client *);

/*
** get_history
**      Return the current client that was using the given
**      nickname within the timelimit. Returns NULL, if no
**      one found...
*/
extern struct Client *get_history(const char *, time_t);

/*
** for debugging...counts related structures stored in whowas array.
*/
extern void count_whowas_memory(int *, unsigned long *);

/* XXX m_whowas.c in modules needs these */
extern struct Whowas *WHOWASHASH[];
extern unsigned int hash_whowas_name(const char* name);
#endif /* INCLUDED_whowas_h */
