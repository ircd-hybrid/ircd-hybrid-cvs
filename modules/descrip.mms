# MMS/MMK Makefile for OpenVMS
# Copyright (c) 2001 Edward Brocklesby
# $Id: descrip.mms,v 1.8 2003/05/13 02:32:13 joshk Exp $

CC=	CC
CFLAGS=	/INCLUDE_DIRECTORY=([-.INCLUDE])/STANDARD=ISOC94
LDFLAGS=

OBJECTS=	M_ACCEPT,M_ADMIN,M_AWAY,M_CAPAB,M_CBURST,-
		M_CLOSE,M_CONNECT,M_DMEM,M_DROP,-
		M_EOB,M_INFO,M_INVITE,M_ISON,M_JOIN,-
		M_KLINE,M_KNOCK,M_LINKS,M_LIST,M_LLJOIN,M_LLNICK,M_LOCOPS,-
		M_LUSERS,M_MOTD,M_NAMES,M_NBURST,-
		M_OPER,M_OPERWALL,M_PASS,M_PING,M_PONG,M_POST,-
		M_REHASH,M_RESTART,M_SET,M_STATS,-
		M_SVINFO,M_TESTLINE,M_TIME,M_TOPIC,M_TRACE,M_UNKLINE,M_USER,-
		M_USERHOST,M_USERS,M_VERSION,M_WALLOPS,M_WHO,M_WHOIS,M_WHOWAS,-
		M_GLINE,M_RESV


ALL : MODULES.OLB($(OBJECTS)) CORE.OLB

CORE.OLB :
	SET DEF [.CORE]
	MMK
	SET DEF [-]

CLEAN : 
	DELETE *.OLB;*
	DELETE *.OBJ;*
