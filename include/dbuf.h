/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  hook.h: A header for the dynamic buffers functions.
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
 *  $Id: dbuf.h,v 7.9 2003/04/19 21:33:48 adx Exp $
 */

#ifndef __DBUF_H_INCLUDED
#define __DBUF_H_INCLUDED

#include "tools.h"

#define DBUF_HEAP_SIZE  512   /* preallocate 0,5 MB */
#define DBUF_BLOCK_SIZE 1024  /* this is also our MTU used for sending */

struct dbuf_block
{
  size_t size;
  char data[DBUF_BLOCK_SIZE];
};

struct dbuf_queue
{
  dlink_list blocks;
  size_t total_size;
};

void dbuf_init(void);
void dbuf_put(struct dbuf_queue *, void *, size_t);
void dbuf_delete(struct dbuf_queue *, size_t);

#define dbuf_length(x) ((x)->total_size)
#define dbuf_clear(x) dbuf_delete(x, dbuf_length(x))

#endif