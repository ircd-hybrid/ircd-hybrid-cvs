/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  class.h: The ircd class management header.
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
 *  $Id: class.h,v 7.23 2003/06/26 04:35:03 db Exp $
 */

#ifndef INCLUDED_class_h
#define INCLUDED_class_h
#include "tools.h"

struct AccessItem;
struct Client;

struct ClassItem
{
  char *class_name;
  int con_freq;
  int ping_freq;
  int max_total;
  int max_local;
  int max_global;
  int max_ident;
  int max_perip;
  long max_sendq;
  int curr_user_count;
};

#define ClassName(x)	((x)->class_name)
#define ConFreq(x)	((x)->con_freq)
#define PingFreq(x)	((x)->ping_freq)
#define MaxTotal(x)	((x)->max_total)
#define MaxGlobal(x)	((x)->max_global)
#define MaxLocal(x)	((x)->max_local)
#define MaxIdent(x)	((x)->max_ident)
#define MaxPerIp(x)	((x)->max_perip)
#define MaxSendq(x)	((x)->max_sendq)
#define CurrUserCount(x) ((x)->curr_user_count)

#define ClassPtr(x)      ((x)->c_class)
#define ConfCurrUserCount(x) (ClassPtr(x)->curr_user_count)
#define ConfMaxTotal(x)  (ClassPtr(x)->max_total)
#define ConfMaxGlobal(x) (ClassPtr(x)->max_global)
#define ConfMaxLocal(x)	 (ClassPtr(x)->max_local)
#define ConfMaxIdent(x)	 (ClassPtr(x)->max_ident)
#define ConfMaxPerIp(x)	 (ClassPtr(x)->max_perip)
#define ConfClassName(x) (ClassPtr(x)->class_name)
#define ConfConFreq(x)   (ClassPtr(x)->con_freq)
#define ConfPingFreq(x)  (ClassPtr(x)->ping_freq)
#define ConfSendq(x)     (ClassPtr(x)->max_sendq)

extern dlink_list class_items;	/* XXX */

extern unsigned long get_sendq(struct Client *);
extern int get_con_freq(struct ClassItem* );
extern const char *get_client_class(struct Client *);
extern int get_client_ping(struct Client *);
extern void check_class(void);
extern void init_class(void);
extern void free_class(struct ClassItem *);
extern void fix_class(struct AccessItem *, struct AccessItem *);
extern struct ClassItem *find_class(const char *);
#endif /* INCLUDED_class_h */
