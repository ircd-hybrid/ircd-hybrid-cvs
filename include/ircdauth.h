/************************************************************************
 *
 *   IRC - Internet Relay Chat, include/ircdauth.h
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: ircdauth.h,v 1.1 1999/09/02 01:23:51 wnder Exp $
 */

#ifndef INCLUDED_ircdauth_h
#define INCLUDED_ircdauth_h

struct Client;

/* ircdauth.c prototypes */

int ConnectToIAuth();
int GenerateClientID(const struct Client *cptr);
int ParseIAuth();

typedef struct IrcdAuthentication IrcdAuth;

#define  NOSOCK        (-1)
#define  MAXPARAMS     15

struct IrcdAuthentication
{
	char hostname[HOSTLEN + 1]; /* hostname of IAuth server */
	int port; /* port for connection */
	int socket; /* socket descriptor for IAuth connection */
};

#endif /* INCLUDED_ircdauth_h */
