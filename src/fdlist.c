/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  fdlist.c: Maintains a list of file descriptors.
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
 *  $Id: fdlist.c,v 7.48 2005/09/18 14:25:13 adx Exp $
 */
#include "stdinc.h"
#include "fdlist.h"
#include "client.h"  /* struct Client */
#include "common.h"
#include "event.h"
#include "ircd.h"    /* GlobalSetOptions */
#include "irc_string.h"
#include "s_bsd.h"   /* comm_setselect */
#include "s_conf.h"  /* ServerInfo */
#include "send.h"
#include "memory.h"
#include "numeric.h"

fde_t *fd_hash[FD_HASH_SIZE];
fde_t *fd_next_in_loop = NULL;
int number_fd = LEAKED_FDS;
int hard_fdlimit;

void
fdlist_init(void)
{
  memset(&fd_hash, 0, sizeof(fd_hash));
  recalc_fdlimit();
}

void
recalc_fdlimit(void)
{
#ifdef _WIN32
  /* this is what WSAStartup() usually returns. Even though they say
   * the value is for compatibility reasons and should be ignored,
   * we actually can create even more sockets... */
  hard_fdlimit = 32767;
#else
  /* allow MAXCLIENTS_MIN clients even at the cost of MAX_BUFFER and
   * some not really LEAKED_FDS */
  int fdmax = getdtablesize();
  hard_fdlimit = IRCD_MAX(fdmax, LEAKED_FDS + MAX_BUFFER + MAXCLIENTS_MIN);
#endif

  if (ServerInfo.max_clients > MAXCLIENTS_MAX)
    ServerInfo.max_clients = MAXCLIENTS_MAX;
}

static inline unsigned int
hash_fd(int fd)
{
#ifdef _WIN32
  return ((((unsigned) fd) >> 2) % FD_HASH_SIZE);
#else
  return (((unsigned) fd) % FD_HASH_SIZE);
#endif
}

fde_t *
lookup_fd(int fd)
{
  fde_t *F = fd_hash[hash_fd(fd)];

  while (F)
  {
    if (F->fd == fd)
      return (F);
    F = F->hnext;
  }

  return (NULL);
}

/* Called to open a given filedescriptor */
void
fd_open(fde_t *F, int fd, int is_socket, const char *desc)
{
  unsigned int hashv = hash_fd(fd);
  assert(fd >= 0);

  F->fd = fd;
  F->comm_index = -1;
  if (desc)
    strlcpy(F->desc, desc, sizeof(F->desc));
  /* Note: normally we'd have to clear the other flags,
   * but currently F is always cleared before calling us.. */
  F->flags.open = 1;
  F->flags.is_socket = is_socket;
  F->hnext = fd_hash[hashv];
  fd_hash[hashv] = F;

  number_fd++;
}

/* Called to close a given filedescriptor */
void
fd_close(fde_t *F)
{
  unsigned int hashv = hash_fd(F->fd);

  if (F == fd_next_in_loop)
    fd_next_in_loop = F->hnext;

  comm_setselect(F, COMM_SELECT_WRITE | COMM_SELECT_READ, NULL, NULL, 0);

  if (F->dns_query != NULL)
  {
    delete_resolver_queries(F->dns_query);
    MyFree(F->dns_query);
  }

#ifdef HAVE_LIBCRYPTO
  if (F->ssl)
    SSL_free(F->ssl);
#endif

  if (fd_hash[hashv] == F)
    fd_hash[hashv] = F->hnext;
  else {
    fde_t *prev;

    /* let it core if not found */
    for (prev = fd_hash[hashv]; prev->hnext != F; prev = prev->hnext)
      ;
    prev->hnext = F->hnext;
  }

  /* Unlike squid, we're actually closing the FD here! -- adrian */
#ifdef _WIN32
  if (F->flags.is_socket)
    closesocket(F->fd);
  else
#endif
    close(F->fd);
  number_fd--;
#ifdef INVARIANTS
  memset(F, '\0', sizeof(fde_t));
#endif
}

/*
 * fd_dump() - dump the list of active filedescriptors
 */
void
fd_dump(struct Client *source_p)
{
  int i;
  fde_t *F;

  for (i = 0; i < FD_HASH_SIZE; i++)
    for (F = fd_hash[i]; F != NULL; F = F->hnext)
      sendto_one(source_p, ":%s %d %s :fd %-5d desc '%s'",
                 me.name, RPL_STATSDEBUG, source_p->name,
                 F->fd, F->desc);
}

/*
 * fd_note() - set the fd note
 *
 * Note: must be careful not to overflow fd_table[fd].desc when
 *       calling.
 */
void
fd_note(fde_t *F, const char *format, ...)
{
  va_list args;

  if (format != NULL)
  {
    va_start(args, format);
    vsnprintf(F->desc, sizeof(F->desc), format, args);
    va_end(args);
  }
  else
    F->desc[0] = '\0';
}

/* Make sure stdio descriptors (0-2) and profiler descriptor (3)
 * always go somewhere harmless.  Use -foreground for profiling
 * or executing from gdb */
#ifndef _WIN32
void
close_standard_fds(void)
{
  int i;

  for (i = 0; i < LOWEST_SAFE_FD; i++)
  {
    close(i);
    if (open(PATH_DEVNULL, O_RDWR) < 0)
      exit(-1); /* we're hosed if we can't even open /dev/null */
  }
}
#endif

void
close_all_fds(void)
{
  int i;

  for (i = 0; i < FD_HASH_SIZE; i++)
    while (fd_hash[i] != NULL)
      fd_close(fd_hash[i]);
}
