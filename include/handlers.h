/*
 * IRC - Internet Relay Chat, include/handlers.h
 * Copyright (C) 1990 Jarkko Oikarinen and
 *                    University of Oulu, Computing Center
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: handlers.h,v 7.24 2000/12/15 02:28:31 db Exp $
 */
#ifndef INCLUDED_handlers_h
#define INCLUDED_handlers_h

/*
 * m_functions execute protocol messages on this server:
 * int m_func(struct Client* cptr, struct Client* sptr, int parc, char* parv[]);
 *
 *    cptr    is always NON-NULL, pointing to a *LOCAL* client
 *            structure (with an open socket connected!). This
 *            identifies the physical socket where the message
 *            originated (or which caused the m_function to be
 *            executed--some m_functions may call others...).
 *
 *    sptr    is the source of the message, defined by the
 *            prefix part of the message if present. If not
 *            or prefix not found, then sptr==cptr.
 *
 *            (!IsServer(cptr)) => (cptr == sptr), because
 *            prefixes are taken *only* from servers...
 *
 *            (IsServer(cptr))
 *                    (sptr == cptr) => the message didn't
 *                    have the prefix.
 *
 *                    (sptr != cptr && IsServer(sptr) means
 *                    the prefix specified servername. (?)
 *
 *                    (sptr != cptr && !IsServer(sptr) means
 *                    that message originated from a remote
 *                    user (not local).
 *
 *
 *            combining
 *
 *            (!IsServer(sptr)) means that, sptr can safely
 *            taken as defining the target structure of the
 *            message in this server.
 *
 *    *Always* true (if 'parse' and others are working correct):
 *
 *    1)      sptr->from == cptr  (note: cptr->from == cptr)
 *
 *    2)      MyConnect(sptr) <=> sptr == cptr (e.g. sptr
 *            *cannot* be a local connection, unless it's
 *            actually cptr!). [MyConnect(x) should probably
 *            be defined as (x == x->from) --msa ]
 *
 *    parc    number of variable parameter strings (if zero,
 *            parv is allowed to be NULL)
 *
 *    parv    a NULL terminated list of parameter pointers,
 *
 *                    parv[0], sender (prefix string), if not present
 *                            this points to an empty string.
 *                    parv[1]...parv[parc-1]
 *                            pointers to additional parameters
 *                    parv[parc] == NULL, *always*
 *
 *            note:   it is guaranteed that parv[0]..parv[parc-1] are all
 *                    non-NULL pointers.
 */

struct Client;

/* unregistered */
extern int mr_capab(struct Client*, struct Client*, int, char**);
extern int mr_nick(struct Client*, struct Client*, int, char**);
extern int mr_error(struct Client*, struct Client*, int, char**);
extern int mr_pong(struct Client*, struct Client*, int, char**);
extern int mr_server(struct Client*, struct Client*, int, char**);
extern int mr_admin(struct Client*, struct Client*, int, char**);

/* registered local clients */
extern int m_users(struct Client *, struct Client *, int, char **);
extern int m_svinfo(struct Client *, struct Client *, int, char **);
extern int m_accept(struct Client*, struct Client*, int, char**);
extern int m_admin(struct Client*, struct Client*, int, char**);
extern int m_away(struct Client*, struct Client*, int, char**);
extern int m_dline(struct Client *,struct Client *,int,char **);
extern int m_dns(struct Client *,struct Client *,int,char **);
extern int m_error(struct Client *,struct Client *,int,char **);
extern int m_gline(struct Client *,struct Client *,int,char **);
extern int m_hash(struct Client *,struct Client *,int,char **);
extern int m_help(struct Client*, struct Client*, int, char**);
extern int m_ignore(struct Client*, struct Client*, int, char**);
extern int m_info(struct Client*, struct Client*, int, char**);
extern int m_invite(struct Client*, struct Client*, int, char**);
extern int m_ison(struct Client*, struct Client*, int, char**);
extern int m_join(struct Client*, struct Client*, int, char**);
extern int m_cjoin(struct Client*, struct Client*, int, char**);
extern int m_kick(struct Client*, struct Client*, int, char**);
extern int m_knock(struct Client*, struct Client*, int, char**);
extern int m_kline(struct Client *,struct Client *,int,char **);
extern int m_links(struct Client*, struct Client*, int, char**);
extern int m_list(struct Client*, struct Client*, int, char**);
extern int m_locops(struct Client *,struct Client *,int,char **);
extern int m_lusers(struct Client*, struct Client*, int, char**);
extern int m_mode(struct Client*, struct Client*, int, char**);
extern int m_motd(struct Client*, struct Client*, int, char**);
extern int m_names(struct Client*, struct Client*, int, char**);
extern int m_nick(struct Client*, struct Client*, int, char**);
extern int m_not_oper(struct Client*, struct Client*, int, char**);
extern int m_notice(struct Client*, struct Client*, int, char**);
extern int m_oper(struct Client*, struct Client*, int, char**);
extern int m_challenge(struct Client*, struct Client*, int, char**);
extern int m_part(struct Client*, struct Client*, int, char**);
extern int m_pass(struct Client*, struct Client*, int, char**);
extern int m_ping(struct Client*, struct Client*, int, char**);
extern int m_pong(struct Client*, struct Client*, int, char**);
extern int m_privmsg(struct Client*, struct Client*, int, char**);
extern int m_proto(struct Client*, struct Client*, int, char**);
extern int m_quit(struct Client*, struct Client*, int, char**);
extern int m_registered(struct Client*, struct Client*, int, char**);
extern int m_stats(struct Client*, struct Client*, int, char**);
extern int m_time(struct Client*, struct Client*, int, char**);
extern int m_topic(struct Client*, struct Client*, int, char**);
extern int m_trace(struct Client*, struct Client*, int, char**);
extern int m_unregistered(struct Client*, struct Client*, int, char**);
extern int m_unsupported(struct Client*, struct Client*, int, char**);
extern int m_user(struct Client*, struct Client*, int, char**);
extern int m_userhost(struct Client*, struct Client*, int, char**);
extern int m_userip(struct Client*, struct Client*, int, char**);
extern int m_version(struct Client*, struct Client*, int, char**);
extern int m_who(struct Client*, struct Client*, int, char**);
extern int m_whois(struct Client*, struct Client*, int, char**);
extern int m_whowas(struct Client*, struct Client*, int, char**);

/* registered local OPERED */
extern int mo_admin(struct Client*, struct Client*, int, char**);
extern int mo_close(struct Client*, struct Client*, int, char**);
extern int mo_connect(struct Client*, struct Client*, int, char**);
extern int mo_die(struct Client*, struct Client*, int, char**);
extern int mo_dline(struct Client*, struct Client*, int, char**);
extern int mo_gline(struct Client*, struct Client*, int, char**);
extern int mo_help(struct Client*, struct Client*, int, char**);
extern int mo_info(struct Client*, struct Client*, int, char**);
extern int mo_kill(struct Client*, struct Client*, int, char**);
extern int mo_kline(struct Client*, struct Client*, int, char**);
extern int mo_links(struct Client*, struct Client*, int, char**);
extern int mo_modlist(struct Client*, struct Client*, int, char**);
extern int mo_modload(struct Client*, struct Client*, int, char**);
extern int mo_notice(struct Client*, struct Client*, int, char**);
extern int mo_oper(struct Client*, struct Client*, int, char**);
extern int mo_operwall(struct Client*, struct Client*, int, char**);
extern int mo_privmsg(struct Client*, struct Client*, int, char**);
extern int mo_rehash(struct Client*, struct Client*, int, char**);
extern int mo_restart(struct Client*, struct Client*, int, char**);
extern int mo_set(struct Client*, struct Client*, int, char**);
extern int mo_squit(struct Client*, struct Client*, int, char**);
extern int mo_stats(struct Client*, struct Client*, int, char**);
extern int mo_time(struct Client*, struct Client*, int, char**);
extern int mo_modunload(struct Client*, struct Client*, int, char**);
extern int mo_testline(struct Client *,struct Client *,int,char **);
extern int mo_trace(struct Client*, struct Client*, int, char**);
extern int mo_unkline(struct Client *,struct Client *,int,char **);
extern int mo_undline(struct Client *, struct Client *, int, char **);
extern int mo_ungline(struct Client *, struct Client *, int, char **);
extern int mo_wallops(struct Client*, struct Client*, int, char**);
extern int mo_htm(struct Client *,struct Client *,int,char **);
extern int mo_quit(struct Client*, struct Client*, int, char**);
extern int mo_part(struct Client*, struct Client*, int, char**);
extern int mo_whois(struct Client*, struct Client*, int, char**);
extern int mo_whowas(struct Client*, struct Client*, int, char**);

/* server */
extern int ms_admin(struct Client*, struct Client*, int, char**);
extern int ms_capab(struct Client*, struct Client*, int, char**);
extern int ms_cburst(struct Client*, struct Client*, int, char**);
extern int ms_connect(struct Client*, struct Client*, int, char**);
extern int ms_drop(struct Client *,struct Client *,int,char **);
extern int ms_eob(struct Client*, struct Client*, int, char**);
extern int ms_error(struct Client*, struct Client*, int, char**);
extern int ms_gline(struct Client*, struct Client*, int, char**);
extern int ms_info(struct Client*, struct Client*, int, char**);
extern int ms_invite(struct Client*, struct Client*, int, char**);
extern int ms_join(struct Client*, struct Client*, int, char**);
extern int ms_kick(struct Client*, struct Client*, int, char**);
extern int ms_kill(struct Client*, struct Client*, int, char**);
extern int ms_kline(struct Client *,struct Client *,int,char **);
extern int ms_links(struct Client*, struct Client*, int, char**);
extern int ms_lljoin(struct Client *,struct Client *,int,char **);
extern int ms_lusers(struct Client*, struct Client*, int, char**);
extern int ms_mode(struct Client*, struct Client*, int, char**);
extern int ms_motd(struct Client*, struct Client*, int, char**);
extern int ms_nick(struct Client*, struct Client*, int, char**);
extern int ms_notice(struct Client*, struct Client*, int, char**);
extern int ms_sjoin(struct Client*, struct Client*, int, char**);
extern int ms_time(struct Client*, struct Client*, int, char**);
extern int ms_names(struct Client*, struct Client*, int, char**);
extern int ms_oper(struct Client*, struct Client*, int, char**);
extern int ms_operwall(struct Client*, struct Client*, int, char**);
extern int ms_part(struct Client*, struct Client*, int, char**);
extern int ms_ping(struct Client*, struct Client*, int, char**);
extern int ms_pong(struct Client*, struct Client*, int, char**);
extern int ms_privmsg(struct Client*, struct Client*, int, char**);
extern int ms_quit(struct Client*, struct Client*, int, char**);
extern int ms_server(struct Client*, struct Client*, int, char**);
extern int ms_squit(struct Client*, struct Client*, int, char**);
extern int ms_stats(struct Client*, struct Client*, int, char**);
extern int ms_topic(struct Client*, struct Client*, int, char**);
extern int ms_trace(struct Client*, struct Client*, int, char**);
extern int ms_version(struct Client*, struct Client*, int, char**);
extern int ms_wallops(struct Client*, struct Client*, int, char**);
extern int ms_who(struct Client*, struct Client*, int, char**);
extern int ms_whois(struct Client*, struct Client*, int, char**);


#endif /* INCLUDED_handlers_h */

