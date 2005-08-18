/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  fileio.h: The file input/output header.
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
 *  $Id: fileio.h,v 7.17 2005/08/18 06:37:41 db Exp $
 */

#ifndef INCLUDED_fileio_h
#define INCLUDED_fileio_h


#include "fdlist.h"

#define FB_EOF  0x01
#define FB_FAIL 0x02

/*
 * FileBuf is a mirror of the ANSI FILE struct, but it works for any
 * file descriptor. FileBufs are allocated when a file is opened with
 * fbopen, and they are freed when the file is closed using fbclose.
 */
typedef struct FileBuf {
  fde_t F;            /* file descriptor */
  char  *endp;        /* one past the end */
  char  *ptr;         /* current read pos */
  char  *pbptr;       /* pointer to push back char */
  int   flags;        /* file state */
  char  buf[BUFSIZ];  /* buffer */
  char  pbuf[BUFSIZ+1]; /* push back buffer */
} FBFILE;

/* open a file and return a FBFILE*, see fopen(3) */
FBFILE *fbopen(const char *, const char *);

/* Positions the file pointer at the beginning of the file */
int fbrewind(FBFILE *);

/* close a file opened with fbopen, see fclose(3) */
void fbclose(FBFILE *);

/* 
 * return the next character from the file, EOF on end of file
 * see fgetc(3)
 */
int fbgetc(FBFILE *);

/*
 * return next string in a file up to and including the newline character
 * see fgets(3)
 */
char *fbgets(char *, size_t, FBFILE *);

/* ungets c to fb see ungetc(3) */
void fbungetc(char, FBFILE *);

/* write a null terminated string to a file, see fputs(3) */
int fbputs(const char *, FBFILE *, size_t);

/* return the status of the file associated with fb, see fstat(3) */
int fbstat(struct stat *, FBFILE *);

/* popen a file. */
FBFILE *fbpopen(const char *, const char *);

int file_open(fde_t *, const char *, int, int);
void file_close(fde_t *);
int save_spare_fd(const char *);

#endif /* INCLUDED_fileio_h */
