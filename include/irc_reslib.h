/*
 * include/res.h (C)opyright 1992 Darren Reed.
 *
 * $Id: irc_reslib.h,v 7.3 2003/05/13 02:36:22 db Exp $
 */
#ifndef INCLUDED_ircdreslib_h
#define INCLUDED_ircdreslib_h

/* OpenBSD and Cygwin (and probably others) don't define these in
 * arpa/nameser.h like FreeBSD and Linux do... */

#ifndef NS_INT16SZ
#define NS_INT16SZ 2
#endif

#ifndef NS_INT32SZ
#define NS_INT32SZ 4
#endif

#ifndef NS_CMPRSFLGS
#define NS_CMPRSFLGS    0xc0
#endif

#ifndef NS_MAXCDNAME
#define NS_MAXCDNAME    255
#endif

/*
 * Inline versions of get/put short/long.  Pointer is advanced.
 */
#define NS_GET16(s, cp) { \
	register u_char *t_cp = (u_char *)(cp); \
	(s) = ((u_int16_t)t_cp[0] << 8) \
	    | ((u_int16_t)t_cp[1]) \
	    ; \
	(cp) += NS_INT16SZ; \
}

#define NS_GET32(l, cp) { \
	register u_char *t_cp = (u_char *)(cp); \
	(l) = ((u_int32_t)t_cp[0] << 24) \
	    | ((u_int32_t)t_cp[1] << 16) \
	    | ((u_int32_t)t_cp[2] << 8) \
	    | ((u_int32_t)t_cp[3]) \
	    ; \
	(cp) += NS_INT32SZ; \
}

#define NS_PUT16(s, cp) { \
	register u_int16_t t_s = (u_int16_t)(s); \
	register u_char *t_cp = (u_char *)(cp); \
	*t_cp++ = t_s >> 8; \
	*t_cp   = t_s; \
	(cp) += NS_INT16SZ; \
}

#define NS_PUT32(l, cp) { \
	register u_int32_t t_l = (u_int32_t)(l); \
	register u_char *t_cp = (u_char *)(cp); \
	*t_cp++ = t_l >> 24; \
	*t_cp++ = t_l >> 16; \
	*t_cp++ = t_l >> 8; \
	*t_cp   = t_l; \
	(cp) += NS_INT32SZ; \
}

/* Function prototypes begin here */
int irc_res_init(void);
int irc_dn_expand(const u_char *msg, const u_char *eom, const u_char *src, char *dst, int dstsiz);
int irc_ns_name_uncompress(const u_char *msg, const u_char *eom, const u_char *src, char *dst, size_t dstsiz);
int irc_ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src, u_char *dst, size_t dstsiz);
int irc_ns_name_ntop(const u_char *src, char *dst, size_t dstsiz);
int irc_dn_comp(const char *src, u_char *dst, int dstsiz, u_char **dnptrs, u_char **lastdnptr);
int irc_dn_skipname(const u_char *ptr, const u_char *eom);
int ns_name_skip(const u_char **ptrptr, const u_char *eom);
int irc_ns_name_pton(const char *src, u_char *dst, size_t dstsiz);
int irc_ns_name_pack(const u_char *src, u_char *dst, int dstsiz, const u_char **dnptrs, const u_char **lastdnptr);
irc_res_mkquery(int op, const char *dname, int class, int type,
                const unsigned char *data,
                int datalen, unsigned char *buf, int buflen);

#endif /* INCLUDED_res_h */

/* $Id: irc_reslib.h,v 7.3 2003/05/13 02:36:22 db Exp $ */
