# MMS/MMK Makefile for OpenVMS
# Copyright (c) 2001 Edward Brocklesby
# $Id: descrip.mms,v 7.15 2003/05/23 17:53:13 joshk Exp $

CC=	CC
CFLAGS=/INCLUDE_DIRECTORY=([-.INCLUDE])/STANDARD=ISOC94/NOWARN -
/DEFINE=(FD_SETSIZE=2048)
LDFLAGS=

DEFAULT : IRCD.EXE

OBJECTS=	BALLOC.OBJ,CHANNEL.OBJ,CHANNEL_MODE.OBJ,CLASS.OBJ,CLIENT.OBJ,-
		CRYPT.OBJ,DBUF.OBJ,DYNLINK.OBJ,EVENT.OBJ,FDLIST.OBJ,FILEIO.OBJ,-
		GETOPT.OBJ,HASH.OBJ,HOOK.OBJ,HOSTMASK.OBJ,IRC_GETADDRINFO.OBJ,-
		IRC_GETNAMEINFO.OBJ,IRC_RES.OBJ,IRC_RESLIB.OBJ,IRC_STRING.OBJ,-
		IRCD.OBJ,IRCD_SIGNAL.OBJ,CSVLIB.OBJ,LIST.OBJ,-
		LISTENER.OBJ,M_ERROR.OBJ,MATCH.OBJ,MD5.OBJ,MEMORY.OBJ,-
		MODULES.OBJ,MOTD.OBJ,NUMERIC.OBJ,PACKET.OBJ,PARSE.OBJ,-
		RESTART.OBJ,RESV.OBJ,RSA.OBJ,S_AUTH.OBJ,S_BSD.OBJ,-
		S_BSD_SELECT.OBJ,S_CONF.OBJ,S_DEBUG.OBJ,S_GLINE.OBJ,S_LOG.OBJ,-
		S_MISC.OBJ,S_SERV.OBJ,S_STATS.OBJ,S_USER.OBJ,SCACHE.OBJ,-
		SEND.OBJ,SNPRINTF.OBJ,SPRINTF_IRC.OBJ,TOOLS.OBJ,WHOWAS.OBJ,-
		SNPRINTF.OBJ

VERSION.C : VERSION.COM
	@- @VERSION

VERSION.OBJ : VERSION.C

IRCD_PARSER_TAB.OBJ : IRCD_PARSER_TAB.C

IRCD_PARSER_TAB.C : IRCD_PARSER.Y
	BISON -d IRCD_PARSER.Y

LEXYY.OBJ : LEXYY.C

LEXYY.C : IRCD_LEXER.L
	FLEX IRCD_LEXER.L

IRCD.EXE : $(OBJECTS) IRCD_PARSER_TAB.OBJ LEXYY.OBJ VERSION.OBJ
	$(LINK)$(LDFLAGS)/EXECUTABLE=IRCD $(OBJECTS), IRCD_PARSER_TAB.OBJ, LEXYY.OBJ, VERSION.OBJ, [-.modules]modules.olb/LIB, [-.modules]core.olb/LIB

CLEAN : 
	DELETE *.OBJ;*
	DELETE *.OLB;*
	DELETE *.EXE;*
