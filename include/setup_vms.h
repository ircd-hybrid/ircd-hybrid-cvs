/* $Id: setup_vms.h,v 7.2 2003/05/28 01:41:22 joshk Exp $
 * Static setup.h for VMS
 */

/* Cygwin requires a different set of headers to compile, so define to 1 if
   compiling on Cygwin. */
#undef CYGWIN

/* Define if this ircd will be an EFnet server. */
#undef EFNET

/* Define to 1 if you have the <crypt.h> header file. */
#undef HAVE_CRYPT_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H

/* Define to 1 if you have the `dlfunc' function. */
#define HAVE_DLFUNC

/* Define to 1 if you have the `dlopen' function. */
#define HAVE_DLOPEN

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H

/* Define to 1 if you have the `EVP_bf_cfb' function. */
#undef HAVE_EVP_BF_CFB

/* Define to 1 if you have the `EVP_cast5_cfb' function. */
#undef HAVE_EVP_CAST5_CFB

/* Define to 1 if you have the `EVP_des_cfb' function. */
#undef HAVE_EVP_DES_CFB

/* Define to 1 if you have the `EVP_des_ede3_cfb' function. */
#undef HAVE_EVP_DES_EDE3_CFB

/* Define to 1 if you have the `EVP_idea_cfb' function. */
#undef HAVE_EVP_IDEA_CFB

/* Define to 1 if you have the `EVP_rc5_32_12_16_cfb' function. */
#undef HAVE_EVP_RC5_32_12_16_CFB

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#undef HAVE_LIBCRYPTO

/* Define to 1 if you have the `z' library (-lz). */
#undef HAVE_LIBZ

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
#undef HAVE_MACH_O_DYLD_H

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP

/* Define if the nanosleep function is available somewhere. */
#undef HAVE_NANOSLEEP

/* shl_load() is available */
#undef HAVE_SHL_LOAD

/* Define to 1 if you have the `snprintf' function. */
#undef HAVE_SNPRINTF

/* Define to 1 if you have the `socketpair' function. */
#undef HAVE_SOCKETPAIR

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the `strlcat' function. */
#undef HAVE_STRLCAT

/* Define to 1 if you have the `strlcpy' function. */
#undef HAVE_STRLCPY

/* Define to 1 if the system has the type `struct addrinfo'. */
#define HAVE_STRUCT_ADDRINFO

/* Define to 1 if the system has the type `struct sockaddr_storage'. */
#undef HAVE_STRUCT_SOCKADDR_STORAGE

/* Define to 1 if you have the <sys/param.h> header file. */
#undef HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/syslog.h> header file. */
#undef HAVE_SYS_SYSLOG_H

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* Define to 1 if you have the `vsnprintf' function. */
#undef HAVE_VSNPRINTF

/* Define if IPv6 support is present and available. */
#undef IPV6

/* Prefix where the ircd is installed. */
#define IRCD_PREFIX "IRCD$BASEDIR:"

/* Maximum no. of clients that can connect to the ircd. */
#define MAX_CLIENTS 200
                    
/* Define this to disable debugging support. */
#undef NDEBUG

/* Nickname length */
#define NICKLEN 9

/* Disable the block allocator. */
#undef NOBALLOC

/* Define if you have no native inet_aton() function. */
#undef NO_INET_ATON

/* Define if you have no native inet_ntop() function. */
#undef NO_INET_NTOP

/* Define if you have no native inet_pton() function. */
#undef NO_INET_PTON

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "<ejb@lythe.org.uk>"

/* Define to the full name of this package. */
#define PACKAGE_NAME "ircd-hybrid"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "ircd-hybrid-7-CURRENT"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ircd-hybrid"

/* Define to the version of this package. */
#define PACKAGE_VERSION "7-CURRENT"

/* Path to /dev/null */
#define PATH_DEVNULL "NL:"

/* RLIMIT value found in sys/resource.h if found */
#undef RLIMIT_FD_MAX

/* This is the type of IO loop we are using */
#define SELECT_TYPE "select"

/* Define to 1 if dynamic modules can't be used. */
#define STATIC_MODULES 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if string.h may be included along with strings.h */
#define STRING_WITH_STRINGS 1

/* String containing extra underscores prepended to symbols loaded from
   modules. */
#define SYMBOL_PREFIX ""

/* Maximum topic length (<=390) */
#define TOPICLEN 390

/* Use kqueue() for I/O loop */
#undef USE_KQUEUE

/* Using sigio futzes around with the ircd - if you want sigio to work, this
   has to be 1. */
#undef USE_SIGIO

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#undef YYTEXT_POINTER

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif

/* If system does not define in_port_t, define it to what it should be. */
#undef in_port_t

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#undef inline

/* If system does not define sa_family_t, define it here. */
#undef sa_family_t

/* If we don't have a real socklen_t, int is good enough. */
#define socklen_t int

/* Broken glibc implementations use __ss_family instead of ss_family. Define
   to __ss_family if true. */
#undef ss_family

/* If system does not define u_int16_t, define a usable substitute. */
#undef u_int16_t

/* If system does not define u_int32_t, define to unsigned int here. */
#undef u_int32_t

#define MAXPATHLEN PATH_MAX
