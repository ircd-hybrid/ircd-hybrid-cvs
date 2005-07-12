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
 *  $Id: fdlist.c,v 7.40 2005/07/12 18:34:42 adx Exp $
 */
#include "stdinc.h"
#include "fdlist.h"
#include "client.h"  /* struct Client */
#include "event.h"
#include "ircd.h"    /* GlobalSetOptions */
#include "irc_string.h"
#include "s_bsd.h"   /* highest_fd */
#include "send.h"
#include "memory.h"
#include "numeric.h"

fde_t *fd_table = NULL;

static void fdlist_update_biggest(int fd, int opening);

/* Highest FD and number of open FDs .. */
int highest_fd = -1; /* Its -1 because we haven't started yet -- adrian */
int number_fd = 0;

static void
fdlist_update_biggest(int fd, int opening)
{ 
  if (fd < highest_fd)
    return;
  assert(fd < HARD_FDLIMIT);

  if (fd > highest_fd)
  {
    /*  
     * assert that we are not closing a FD bigger than
     * our known biggest FD
     */
    assert(opening);
    highest_fd = fd;
    return;
  }

  /* if we are here, then fd == Biggest_FD */
  /*
   * assert that we are closing the biggest FD; we can't be
   * re-opening it
   */
  assert(!opening);
  while (highest_fd >= 0 && !fd_table[highest_fd].flags.open)
    highest_fd--;
}

void
fdlist_init(void)
{
  static int initialized = 0;

  if (!initialized)
  {
      /* Since we're doing this once .. */
      fd_table = MyMalloc((HARD_FDLIMIT + 1) * sizeof(fde_t));
      initialized = 1;
  }
}

/* Called to open a given filedescriptor */
void
fd_open(int fd, unsigned int type, const char *desc, void *ssl)
{
  fde_t *F = &fd_table[fd];
  assert(fd >= 0);
  
  if (F->flags.open)
    {
#ifdef NOTYET
      debug(51, 1) ("WARNING: Closing open FD %4d\n", fd);
#endif
      fd_close(fd);
    }
  assert(!F->flags.open);
#ifdef NOTYET
  debug(51, 3) ("fd_open FD %d %s\n", fd, desc);
#endif
  F->fd = fd;
  F->type = type;
  F->flags.open = 1;
#ifdef NOTYET
  F->defer.until = 0;
  F->defer.n = 0;
  F->defer.handler = NULL;
#endif
  fdlist_update_biggest(fd, 1);
  F->comm_index = -1;
  F->list = FDLIST_NONE;
  if (desc)
    strlcpy(F->desc, desc, sizeof(F->desc));
  number_fd++;
#ifdef HAVE_LIBCRYPTO
  F->ssl = (SSL *)ssl;
#endif
}

/* Called to close a given filedescriptor */
void
fd_close(int fd)
{
  fde_t *F = &fd_table[fd];
  assert(F->flags.open);

  /* All disk fd's MUST go through file_close() ! */
  assert(F->type != FD_FILE);
  if (F->type == FD_FILE)
    {
      assert(F->read_handler == NULL);
      assert(F->write_handler == NULL);
    }
#ifdef NOTYET
  debug(51, 3) ("fd_close FD %d %s\n", fd, F->desc);
#endif
  comm_setselect(fd, FDLIST_NONE, COMM_SELECT_WRITE|COMM_SELECT_READ,
		 NULL, NULL, 0);

  if (F->dns_query != NULL)
  {
    delete_resolver_queries(F->dns_query);
    MyFree(F->dns_query);
    F->dns_query = NULL;
  }

#ifdef HAVE_LIBCRYPTO
  if (F->ssl)
  {
    SSL_free(F->ssl);
    F->ssl = NULL;
  }
#endif

  F->flags.open = 0;
  fdlist_update_biggest(fd, 0);

  number_fd--;
  memset(F, '\0', sizeof(fde_t));

  /* Unlike squid, we're actually closing the FD here! -- adrian */
  close(fd);
}

/*
 * fd_dump() - dump the list of active filedescriptors
 */
void
fd_dump(struct Client *source_p)
{
  int i;

  for (i = 0; i <= highest_fd; i++)
  {
    if (!fd_table[i].flags.open)
      continue;

    sendto_one(source_p, ":%s %d %s :fd %-5d desc '%s'",
               me.name, RPL_STATSDEBUG, source_p->name,
               i, fd_table[i].desc);
  }
}

/*
 * fd_note() - set the fd note
 *
 * Note: must be careful not to overflow fd_table[fd].desc when
 *       calling.
 */
void
fd_note(int fd, const char *format, ...)
{
  va_list args;
  
  if (format)
    {
      va_start(args, format);
      vsnprintf(fd_table[fd].desc, FD_DESC_SZ, format, args);
      va_end(args);
    }
  else
    fd_table[fd].desc[0] = '\0';
}

