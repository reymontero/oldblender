/**
 * $Id:$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 1997 by Ton Roosendaal, Frank van Beek and Joeri Kassenaar.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/*

**  Subsystem:   User Level mount for CD-ROM modified for magneto-optic
**  File Name:   mountmo.c
**                                                        
** This software is Copyright (c) 1991 by Kent Landfield.
** based on cdmount modified by Alan Hewat for magneto-optic drive
**
** Permission is hereby granted to copy, distribute or otherwise 
** use any part of this package as long as you do not try to make 
** money from it or pretend that you wrote it.  This copyright 
** notice must be maintained in any copy made.
**
** Use of this software constitutes acceptance for use in an AS IS 
** condition. There are NO warranties with regard to this software.  
** In no event shall the author be liable for any damages whatsoever 
** arising out of or in connection with the use or performance of this 
** software.  Any use of this software is at the user's own risk.
**
**  If you make modifications to this software that you feel 
**  increases it usefulness for the rest of the community, please 
**  email the changes, enhancements, bug fixes as well as any and 
**  all ideas to me. This software is going to be maintained and 
**  enhanced as deemed necessary by the community.
**              
**              Kent Landfield
**              sparky!kent
**              kent@sparky.imd.sterling.com
**
**              Alan Hewat
**              hewat@ill.fr
**
** Use the following commands to clean make scsicmd.
** Then copy tis executable binary to /usr/local/bin
**
**        cc scsicmd.c -O -lds -o scsicmd

** Modified by Frank van Beek (frank@neogeo.nl) and changed name to 
** scsicmd. Part of the functions have been moved to a csh-script that
** unmounts / mounts the mo drive on all hosts in a network.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dslib.h>
#include <stdlib.h>
#include <fcntl.h>

void
ejectmedia( struct dsreq *dsp )
{
	fillg0cmd( dsp, (uchar_t *) CMDBUF( dsp ),  G0_LOAD, 1, 0, 0, 2, 0 );
	filldsreq( dsp, NULL, 0, DSRQ_READ|DSRQ_SENSE );
	doscsireq( getfd( dsp ), dsp );
}

void
closecd( struct dsreq *dsp )
{
	fillg0cmd( dsp, (uchar_t *) CMDBUF( dsp ),  G0_LOAD, 1, 0, 0, 3, 0 );
	filldsreq( dsp, NULL, 0, DSRQ_READ|DSRQ_SENSE );
	doscsireq( getfd( dsp ), dsp );
}

void
prevent_removal( struct dsreq *dsp )
{
	fillg0cmd( dsp, (uchar_t *) CMDBUF( dsp ), G0_PREV, 0, 0, 0, 1, 0 );
	filldsreq( dsp, NULL, 0, DSRQ_READ|DSRQ_SENSE );
	doscsireq( getfd( dsp ), dsp );
}

void
allow_removal( struct dsreq *dsp )
{
	fillg0cmd( dsp, (uchar_t *) CMDBUF( dsp ), G0_PREV, 0, 0, 0, 0, 0 );
	filldsreq( dsp, NULL, 0, DSRQ_READ|DSRQ_SENSE );
	doscsireq( getfd( dsp ), dsp );
}


int
scsitest( struct dsreq *dsp )
{
	fillg0cmd( dsp, (uchar_t *) CMDBUF( dsp ), G0_TEST, 0, 0, 0, 0, 0 );
	filldsreq( dsp, NULL, 0, DSRQ_READ|DSRQ_SENSE );
	doscsireq( getfd( dsp ), dsp );
}

static int
eject( char *fsname )
{
	dsreq_t *dsp;
	int ret, retries;

	dsp = dsopen(fsname, O_RDONLY);
	if (!dsp) {
		perror( fsname );
		return 2;
	}
	allow_removal( dsp );
	ejectmedia( dsp );
	ret = (STATUS( dsp ) == STA_GOOD ? 0 : 3);
	if (ret != 0)
		fprintf( stderr, "Can't eject media for %s\n", fsname );
	dsclose( dsp );
	return (ret);
}

static int
close_cd( char *fsname )
{
	dsreq_t *dsp;
	int ret, retries;

	dsp = dsopen(fsname, O_RDONLY);
	if (!dsp) {
		perror( fsname );
		return 2;
	}
	allow_removal( dsp );
	closecd( dsp );
	ret = (STATUS( dsp ) == STA_GOOD ? 0 : 3);
	if (ret != 0)
		fprintf( stderr, "Can't close media for %s\n", fsname );
	dsclose( dsp );
	return (ret);
}

int lock( char *fsname )
{
	dsreq_t *dsp;

	dsp = dsopen(fsname, O_RDONLY);
	if (!dsp) {
		perror( fsname );
		return 2;
	}
	prevent_removal( dsp );
	dsclose( dsp );
	return (0);
}

int status( char *fsname )
{
	dsreq_t *dsp;
	int ret;
	
	dsp = dsopen(fsname, O_RDONLY);
	if (!dsp) {
		perror( fsname );
		return -1;
	}
	
	scsitest(dsp);
	ret = STATUS( dsp ) - STA_GOOD;
	dsclose( dsp );
	return (ret);
}

int print( char *fsname )
{
	dsreq_t *dsp;
	int ret;
	
	dsp = dsopen(fsname, O_RDONLY);
	if (!dsp) {
		perror( fsname );
		return -1;
	}
	
	printf("scsitest: %d \n", scsitest(dsp));
	/* 2 = geen disk */
	/* 0 = OK */
	
	printf("ret: %d\n", RET(dsp));
	printf("status: %d\n", STATUS(dsp));
	printf("flags: %d\n", FLAGS(dsp));
	
	dsclose( dsp );
	return (ret);
}

/* *********************************************************************** */

void usage(char * progname)
{
    (void) fprintf(stderr, "\nusage: %s lock | eject | status | close /dev/scsi/*\n", progname);
	exit(1);
}

int main(int argc, char ** argv)
{
	int ret = 1;
	
	if (argc < 3) usage(argv[0]);

	if (strcmp(argv[1], "eject") == 0) {
		ret = eject(argv[2]);
	} else if (strcmp(argv[1], "close") == 0) {
		ret = close_cd(argv[2]);
	} else if (strcmp(argv[1], "lock") == 0) {
		ret = lock(argv[2]);
	} else if (strcmp(argv[1], "status") == 0) {
		ret = status(argv[2]);
		printf("%d\n", ret);
	} else if (strcmp(argv[1], "print") == 0) {
		ret = print(argv[2]);
	} else usage(argv[0]);
	
	return(ret);
}

