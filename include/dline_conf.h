/* dline_conf.h  -- lets muse over dlines, shall we?
 *
 * $Id: dline_conf.h,v 7.5 2001/01/18 21:46:47 db Exp $ 
 */

#ifndef INCLUDED_dline_conf_h
#define INCLUDED_dline_conf_h

struct Client;
struct ConfItem;

extern void clear_Dline_table();
extern void zap_Dlines();
extern void add_Dline(struct ConfItem *conf_ptr);
extern void add_ip_Kline(struct ConfItem *conf_ptr);

extern void add_Eline(struct ConfItem *conf_ptr);

extern struct ConfItem *match_Dline(unsigned long ip);
extern struct ConfItem* match_ip_Kline(unsigned long ip, const char* name);

extern void report_dlines(struct Client *sptr);
extern void report_ip_Klines(struct Client *sptr);

#endif /* INCLUDED_dline_conf_h */


