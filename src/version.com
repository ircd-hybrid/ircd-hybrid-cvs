$!
$! VERSION.COM
$! Written By:  Robert Alan Byer
$!              byer@mail.ourservers.net
$!
$!
$! Tell The User What's Going On.
$!
$ WRITE SYS$OUTPUT ""
$ WRITE SYS$OUTPUT "Extracting SYS$DISK:[]VERSION.C"
$ WRITE SYS$OUTPUT ""
$!
$! Assign A Generation Number.
$!
$ GENERATION = 0
$
$ OPEN/ERROR=NOOPEN IN SYS$DISK:[]GENERATION.
$ READ/END_OF_FILE=IGNORE/ERROR=IGNORE IN GENERATION
$ IGNORE:
$ CLOSE IN
$ NOOPEN:
$ GENERATION = GENERATION + 1
$ OPEN/WRITE OUT SYS$DISK:[]GENERATION.
$ WRITE OUT GENERATION
$ CLOSE OUT
$ 
$!
$! Let's Get Our DECNet Node Name.
$!
$ NODE_NAME = F$GETSYI("NODENAME")
$!
$! Let's Get Our OpenVMS Version Number.
$!
$ VERSION_NUMBER = F$GETSYI("VERSION")
$!
$! Let's Get Our Hardware Type.
$!
$ HARDWARE_TYPE = F$GETSYI("HW_NAME")
$!
$! Extract Just The Date For The Creation Date.
$!
$ CREATION_DATE = F$EXTRACT(0,F$LOCATE(" ",F$TIME()),F$TIME())
$!
$! Let's Open Our Output File.
$!
$ OPEN/WRITE FILE SYS$DISK:[]VERSION.C
$!
$! Let's Put Stuff In The File.
$!
$ WRITE FILE "/*"
$ WRITE FILE " *   IRC - Internet Relay Chat, src/version.c"
$ WRITE FILE " *   Copyright (C) 1990 Chelsea Ashley Dyerman"
$ WRITE FILE " *"
$ WRITE FILE " *   This program is free software; you can redistribute it and/or modify"
$ WRITE FILE " *   it under the terms of the GNU General Public License as published by"
$ WRITE FILE " *   the Free Software Foundation; either version 1, or (at your option)"
$ WRITE FILE " *   any later version."
$ WRITE FILE " *"
$ WRITE FILE " *   This program is distributed in the hope that it will be useful,"
$ WRITE FILE " *   but WITHOUT ANY WARRANTY; without even the implied warranty of"
$ WRITE FILE " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
$ WRITE FILE " *   GNU General Public License for more details."
$ WRITE FILE " *"
$ WRITE FILE " *   You should have received a copy of the GNU General Public License"
$ WRITE FILE " *   along with this program; if not, write to the Free Software"
$ WRITE FILE " *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."
$ WRITE FILE " */"
$ WRITE FILE ""
$ WRITE FILE "/*"
$ WRITE FILE " * This file is generated by version.com. Any changes made will go away."
$ WRITE FILE " */"
$ WRITE FILE ""
$ WRITE FILE "#include ""patchlevel.h"""
$ WRITE FILE "#include ""serno.h"""
$ WRITE FILE ""
$ WRITE FILE "char *generation = """, GENERATION, """;
$ WRITE FILE "char *creation = """, CREATION_DATE, """;
$ WRITE FILE "char *platform = ""OpenVMS ",NODE_NAME," ",VERSION_NUMBER,HARDWARE_TYPE,""";
$ WRITE FILE "char *ircd_version = PATCHLEVEL;
$ WRITE FILE "char *serno = SERIALNUM;
$ WRITE FILE ""
$ WRITE FILE "char *infotext[] ="
$ WRITE FILE "{"
$ WRITE FILE " ""$package --"","
$ WRITE FILE "  ""Based on the original code written by Jarkko Oikarinen"","
$ WRITE FILE "  ""Copyright 1988, 1989, 1990, 1991 University of Oulu, Computing Center"","
$ WRITE FILE "  """","
$ WRITE FILE "  ""This program is free software; you can redistribute it and/or"","
$ WRITE FILE "  ""modify it under the terms of the GNU General Public License as"","
$ WRITE FILE "  ""published by the Free Software Foundation; either version 1, or"","
$ WRITE FILE "  ""(at your option) any later version."","
$ WRITE FILE "  """","
$ WRITE FILE "};"
$!
$! Close The Output File.
$!
$ CLOSE FILE
$!
$! That's All, Time To Exit.
$!
$ EXIT