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

 * File: 	contcapt.c
 *
 * Usage:	contcapt [-b numofbuffers] [-c numofframes] [-d] [-D]
 *                       [-f] [-F] [-g wxh] [-i frames] [-m] [-n devicenum]
 *			 [-r rate] [-t]	[-v videosource] [-w] [-z n/d] [-h]
 *
 * Description:	Contcapt demonstrates continuous capture using buffering 
 *				and sproc.
 *
 * Functions:	SGI Video Library functions used (see other modules)
 *
 *		vlOpenVideo()
 *		vlGetDeviceList()
 *		vlGetNode()
 *		vlCreatePath()
 *		vlGetDevice()
 *		vlSetupPaths()
 *		vlRegisterHandler()
 *		vlSelectEvents()
 *		vlSetControl()
 *		vlGetControl()
 *		vlRegisterBuffer()
 *		vlAddCallback()
 *		vlBeginTransfer()
 *		vlMainLoop()
 *		vlEndTransfer()
 *		vlDeregisterBuffer()
 *		vlDestroyPath()
 *		vlCloseVideo()
 *		vlPerror()
 *		vlStrError()
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>
#include <sys/time.h>

#define X_H
#define _XLIB_H_

#include <gl/gl.h>
#include <gl/device.h>
#include <gl/image.h>

#include <dmedia/vl.h>
#include <local/iff.h>

#define VL_PACKING_INVALID	-1
#define Debug			if (debug) printf
#define TIME_ARRAY_SIZE		400	/* MUST be divisible by 5! */
#define TIME_GRANULARITY	1000

#define BUTX	256
#define BUTY	256 + 50

#define VECX	256 + Width
#define VECY	256 

#define RAMPBASE (512 + 128)
#define RAMPSIZE 16
#define RAMPSTEP (255 / (RAMPSIZE - 1))

Matrix uvmat, ytmat;
Scoord uvl, uvr, uvb, uvt;
Scoord ytl, ytr, ytb, ytt;
float yofs = 0.5;
int Width = 768;

long barcolors[] = {
    0xffffff, 0x00c0c0, 0xc0c000, 0x00c000, 0xc000c0, 0x0000c0, 0xc00000,0x000000};

/*
 *  Global variables
 */

char *cmdargs = "b:c:dDfFg:hi:mn:r:tv:wz:";

char *usage_m =
"usage: %s %s\n"
"where arguments may be:\n"
"	-b n	use n buffers\n"
"	-c n	capture n frames\n"
"	-d	turn off double buffer mode\n"
"	-D	turn on debug mode (displays more information)\n"
"	-f	fast mode (no display)\n"
"	-F	specifies field (or non-interleaved) mode\n"
"	-g wxh	frame geometry (ex. 640x480)\n"
"	-i n	report statistics every n frames\n"
"	-m	monochrome input\n"
"	-n	device number\n"
"	-r n	set frame rate to n\n"
"	-t	turn on timing statistics gathering\n"
"	-v	video source (depends on hardware; see release notes)\n"
"	-w	32 bit 'wide' RGB pixels\n"
"	-z d/d	set zoom factor to fraction 'd/d'\n"
"	-h	this help message\n"
;

char *		_progName;
int		buffers = 1;
int		packing = VL_PACKING_YVYU_422_8;
int		totalCount = 10000000;
int		debug = 0;
int		devicenum = -1;
VLDev		deviceId;
int		double_buffer = 1;
int		fast = 0;
int		fieldmode = 0;
int		frameCount;
int		frameInterval = -1;
int		framenumber = 0;
int		lasttime = 0;
int		rate = 0;
int		secs;
int		time_array[TIME_ARRAY_SIZE];
int		time_on = 0;
int		xsize = 0;
int		ysize = 0;
int		zoom_num = 1, zoom_denom = 1;
int		vidsrc = VL_ANY;
struct timeval	tv_save;
VLPath		vlPath;
VLServer	vlSvr;
VLNode		drn;
VLNode		src;
VLBuffer	transferBuf;


/*
 * Function protoypes 
 */

void	 	ProcessEvent(VLServer, VLEvent *, void *);
void		ProcessGlEvent(int, void *);
void		exitcapture(void);
void		initStatistics(void);
void		reportStatistics(void);
void		getcmdargs(int argc, char **argv);
char *		packing_name (int packtype);


/* Get the start time (for frame rate measurement) */
void
initStatistics(void)
{
	if (!debug) return;

	gettimeofday(&tv_save);
}

/* Calculate and display the frame rate */
void
reportStatistics(void)
{
	double rate;
	struct timeval tv;
	int delta_t;

	gettimeofday(&tv);

	/* Delta_t in microseconds */
	delta_t = (tv.tv_sec*1000 + tv.tv_usec/1000) -
	    (tv_save.tv_sec*1000 + tv_save.tv_usec/1000);

	rate = frameCount*1000.0/delta_t;

	printf("got %5.2f %s/sec\n", rate,  fieldmode? "fields" : "frames");
	tv_save.tv_sec = tv.tv_sec;
	tv_save.tv_usec = tv.tv_usec;
}

char *
packing_name (int packtype)
{
	switch (packtype)
	{
	case VL_PACKING_RGB_8:
		return "RGB";
		break;

	case VL_PACKING_RGBA_8:
		return "RGBA";
		break;

	case VL_PACKING_RBG_323:
		return "Starter Video RGB8";
		break;

	case VL_PACKING_RGB_332_P:
		return "RGB332";
		break;

	case VL_PACKING_Y_8_P:
		return "8 Bit Greyscale";
		break;

	default:
		return "???";
		break;
	}
}

/* Parse the command line args */
void
getcmdargs(int argc, char **argv)
{
	int c;
	int len, xlen, ylen;
	char *numbuf;
	char *xloc;

	while ((c = getopt(argc, argv, cmdargs)) != EOF)
	{
		switch (c)
		{

		case 'b':
			buffers = atoi(optarg);
			break;

		case 'c':
			totalCount = atoi(optarg);
			break;

		case 'D':
			debug = 1;
			break;

		case 'd':
			double_buffer = 0;
			break;

		case 'f':
			packing =VL_PACKING_YVYU_422_8;
			break;

		case 'F':
			fieldmode = 1;
			break;

		case 'g':
			len = strlen(optarg);
			xloc = strchr(optarg, 'x');
			if (!xloc)
			{
				printf("Error: invalid geometry format, using default size\n");
				break;
			}
			xlen = len - strlen(xloc);
			if (xlen < 1 || xlen > 4)
			{
				printf("Error: invalid x size, using default size\n");
				break;
			}
			numbuf = strtok(optarg,"x");
			xsize = atoi(numbuf);

			ylen = len - xlen -1;
			if (ylen < 1 || ylen > 4)
			{
				printf("Error: invalid y size, using default size\n");
				break;
			}
			numbuf = strtok(NULL,"");
			ysize = atoi(numbuf);
			break;

		case 'i':
			frameInterval = atoi(optarg);
			printf("frameInterval = %d\n", frameInterval);
			break;

		case 'm':
			packing = VL_PACKING_Y_8_P;
			break;

		case 'n':
			devicenum = atoi(optarg);
			printf("using device number: %d\n", devicenum);
			break;

		case 'r':
			rate = atoi(optarg);
			break;

		case 't':
			time_on = 1;
			break;

		case 'v':
			vidsrc = atoi(optarg);
			break;

		case 'w':
			packing = VL_PACKING_RGB_8;
			break;

		case 'z':
			if (sscanf(optarg,"%d/%d", &zoom_num, &zoom_denom) != 2 ||
			    !zoom_num || !zoom_denom)
			{
				fprintf(stderr, "%s: ERROR: zoom format <num>/<denom>",
				    _progName);
				exit(1);
			}
			break;

		case 'h':
		default:
			fprintf(stderr, usage_m, _progName, cmdargs);
			exit(0);
			break;
		}
	}
}

#define WHITELEVEL 235
#define BLACKLEVEL 16

draw_vector(ushort * fbuf)
{
	static float ybuf[768], uvbuf[768 / 2][2];
	float f0, f1, f2;
	long x, y, i, j, changed, col;
	float co[2];
	char string[10], ret;
	
	pushviewport();
	reshapeviewport();
	color(RAMPBASE);
	clear();
	popviewport();
	
	y = ((1.0 - yofs) * 575.0) + 0.5;
	fbuf += Width * y;
	sprintf(string, "%3d", y);

	/* scanline inlezen */

	for (x = 0; x < Width; x += 2){
		ybuf[x] = fbuf[x] & 0xff;
		ybuf[x+1] = fbuf[x + 1] & 0xff;
		
		uvbuf[x / 2][0] = fbuf[x] & 0xff;
		uvbuf[x / 2][1] = fbuf[x + 1] & 0xff;
		uvbuf[x / 2][0] = (fbuf[x] >> 8) & 0xff;
		uvbuf[x / 2][1] = (fbuf[x+1] >> 8) & 0xff;
	}

	pushmatrix();
	pushviewport();

	viewport(uvl, uvr, uvb, uvt);
	loadmatrix(uvmat);

	/* ylijn tekenen en tekstje printen */

	color(WHITE);
	bgnline();
	co[0] = 128.0;
	co[1] = (yofs * 256.0) - 128.0;
	v2f(co);
	co[0] = 100.0;
	v2f(co);
	endline();

	cmov2s(100, -120);
	charstr(string);

	/* u & v tekenen */
	color(RAMPBASE + 15);
	linesmooth(SML_ON);
	bgnline();
	for (x = 0; x < 360; x++){
		co[0] = uvbuf[x][0] - 128.0;
		co[1] = uvbuf[x][1] - 128.0;
		v2f(co);
	}
	endline();
	linesmooth(SML_OFF);
	
	/* vierkantjes voor u & v tekenen */
	for (i = 1; i <= 7 ; i++){
		col = 0;
		if (i & 1) col += 0x0000c0;
		if (i & 2) col += 0x00c000;
		if (i & 4) col += 0xc00000;
		col = colcspace(col,rgbbeta);
		x = (col >> 16) & 0xff;
		y = col & 0xff;
		x -= 128;
		y -= 128;
		color(i);
		recti(x - 3, y - 3, x + 3, y + 3);
	}

	viewport(ytl, ytr, ytb, ytt);
	loadmatrix(ytmat);

	/* y - lijnen tekenen */
	
	color(RAMPBASE + 4);
	for (x = 0 ; x <= 8 ; x++){
		bgnline();
			co[0] = x * Width / 8;
			co[1] = 1000;
			v2f(co);
			co[1] = -1000;
			v2f(co);			
		endline();
	}


	
	for (x = 0 ; x < 8 ; x++){
		col = colcspace(barcolors[x], rgbbeta);
		col = (col >> 8) & 0xff;
		col = (col * (WHITELEVEL - BLACKLEVEL) / 255.0) + BLACKLEVEL + 0.5;
		bgnline();
			co[0] = 1000;
			co[1] = col;
			v2f(co);
			co[0] = -1000;
			v2f(co);			
		endline();
	}
	
	color(WHITE);
		bgnline();
			co[0] = 1000;
			co[1] = BLACKLEVEL;
			v2f(co);
			co[0] = -1000;
			v2f(co);			
		endline();
		bgnline();
			co[0] = 1000;
			co[1] = WHITELEVEL;
			v2f(co);
			co[0] = -1000;
			v2f(co);			
		endline();

/*	color(BLUE);
	bgnline();
	for (x = 0; x < Width; x++){
		co[0] = x;
		co[1] = buf[x][0];
		v2f(co);
	}
	endline();

	color(RED);
	bgnline();
	for (x = 0; x < Width; x++){
		co[0] = x;
		co[1] = buf[x][2];
		v2f(co);
	}
	endline();
*/
	linesmooth(SML_ON);
	color(RAMPBASE + 15);
	bgnline();
	for (x = 0; x < Width; x++){
		co[0] = x;
		co[1] = ybuf[x];
		v2f(co);
	}
	endline();
	linesmooth(SML_OFF);
	
	swapbuffers();

	popviewport();
	popmatrix();
		
	/* wacht totdat grab gedaan is */
}

main(int argc, char **argv)
{
	VLDevList devlist;
	char *deviceName;
	char window_name[128];
	char *xloc;
	long win;
	int c, i;
	int len, xlen, ylen;
	VLControlValue val;

	_progName = argv[0];

	/* Parse command line args */
	getcmdargs(argc, argv);

	/* Clear the array that will hold timestamps of captured frames */
	if (time_on)
		for (c = 0; c < TIME_ARRAY_SIZE; c++)
			time_array[c] = 0;

	/* Connect to the video daemon */
	if (!(vlSvr = vlOpenVideo("")))
	{
		printf("couldn't open video\n");
		exit(1);
	}

	/* Get the list of the devices that the daemon supports */
	if (vlGetDeviceList(vlSvr, &devlist) < 0)
	{
		printf("error getting device list: %s\n",vlStrError(vlErrno));
		exit(1);
	}

	/* Set up a source node on the specified video source */
	Debug("using video source number: %d\n", vidsrc);
	src = vlGetNode(vlSvr, VL_SRC, VL_VIDEO, vidsrc);

	/* Set up a drain node in memory */
	drn = vlGetNode(vlSvr, VL_DRN, VL_MEM, VL_ANY);

	/* 
     * Create a path from video to memory. 
     * If the user did not specify a device on the command        
     * line , i.e. devicenum = -1, use the first device 
     * that supports such a path. 
     */
	if (devicenum == -1) /* No user-specified device */
	{
		int i;

		vlPath = vlCreatePath(vlSvr, VL_ANY, src, drn);
		if (vlPath < 0)
		{
			printf("can't create path: %s\n", vlStrError(vlErrno));
			exit(1);
		}
		/* Get the device number for future use */
		deviceId = vlGetDevice(vlSvr, vlPath);
		for (i=0; i<devlist.numDevices; i++)
			if (devlist.devices[i].dev == deviceId) {
				devicenum = i;
				break;
			}
	}
	else /* User specified a device */
	{
		deviceId = devlist.devices[devicenum].dev;
		if ((vlPath = vlCreatePath(vlSvr, deviceId, src, drn)) < 0) {
			vlPerror("vlCreatePath");
			exit(1);
		}
	}

	/* Set up the hardware for and define the usage of the path */
	if (vlSetupPaths(vlSvr, (VLPathList)&vlPath, 1, VL_SHARE, VL_SHARE) < 0)
	{
		printf ("could not setup a vid to mem path\n");
		exit(1);
	}

	/* Get the name of the device we're using */
	deviceName = devlist.devices[devicenum].name;

	/* Set the zoom ratio */
	val.fractVal.numerator = zoom_num;
	val.fractVal.denominator = zoom_denom;
	vlSetControl(vlSvr, vlPath, drn, VL_ZOOM, &val);

	/* Set the frame rate */
	if (rate)
	{
		val.fractVal.numerator = rate;
		val.fractVal.denominator = 1;
		vlSetControl(vlSvr, vlPath, drn, VL_RATE, &val);
	}
	vlGetControl(vlSvr, vlPath, drn, VL_RATE, &val);
	Debug("frame rate = %d/%d\n", val.fractVal.numerator,
	    val.fractVal.denominator);

	/* 
     * Specify what vlPath-related events we want to receive.
     * In this example we only want transfer complete, transfer
     * failed and stream preempted events.
     */
	vlSelectEvents(vlSvr, vlPath, VLTransferCompleteMask|
	    VLStreamPreemptedMask|VLTransferFailedMask);

	if (vlGetControl(vlSvr, vlPath, drn, VL_CAP_TYPE, &val) < 0)
		vlPerror("Getting VL_CAP_TYPE");
	Debug("cap type = %d\n", val.intVal);

	/* Set up for non-interleaved frame capture */
	if (fieldmode)
	{
		val.intVal = VL_CAPTURE_NONINTERLEAVED;
		if (vlSetControl(vlSvr, vlPath, drn, VL_CAP_TYPE, &val) < 0)
			vlPerror("Setting VL_CAP_TYPE");

		/* Make sure non-interleaved capture is supported by the hardware */
		if (vlGetControl(vlSvr, vlPath, drn, VL_CAP_TYPE, &val) < 0)
			vlPerror("Geting VL_CAP_TYPE");
		if (val.intVal != VL_CAPTURE_NONINTERLEAVED)
			fprintf(stderr,"Capture type not set to NONINTERLEAVED: %d\n",
			    val.intVal);
		Debug("Field (non-interleaved) mode\n");
		vlGetControl(vlSvr, vlPath, drn, VL_RATE, &val);
		Debug("field rate = %d/%d\n", val.fractVal.numerator,
		    val.fractVal.denominator);
	}

	/* Set the video geometry */
	if (xsize)
	{
		val.xyVal.x = xsize;
		val.xyVal.y = ysize;
		vlSetControl(vlSvr, vlPath, drn, VL_SIZE, &val);
	}
	/* Make sure this size is supported by the hardware */
	vlGetControl(vlSvr, vlPath, drn, VL_SIZE, &val);
	Width = xsize = val.xyVal.x;
	ysize = val.xyVal.y;
	Debug("grabbing size %dx%d\n", val.xyVal.x, val.xyVal.y);

	/* Set the window to the same geometry as the video data */
	prefsize(VECX, BUTY);
	sprintf (window_name, "Vectorscoop %s", deviceName);
	/* Create a graphics window that will display the video data */
	win = winopen(window_name);

	/* Set up to double buffer the data */
	doublebuffer();
	
	/* Set the graphics to RGB mode */
	cmode();

	/* Allow these key presses, mouseclicks, etc to be entered in the event queue */
	qdevice(DKEY);
	qdevice(QKEY);
	qdevice(ESCKEY);
	qdevice(WINSHUT);
	qdevice(WINQUIT);

	/* Set up the graphics subsystem */
	gconfig();
	
	reshapeviewport();
	color(0);
	clear();
	swapbuffers();
	color(0);
	clear();
	subpixel(TRUE);
	
	for (i = 0; i < RAMPSIZE; i++) mapcolor(RAMPBASE + i, i * RAMPSTEP, i * RAMPSTEP, i * RAMPSTEP);
	
	/* matrixen opzetten */
			viewport(0, VECX - 1, 0, VECY - 1);
			ortho2(-0.5,VECX-0.5,-0.5,VECY-0.5);

			pushmatrix();
			pushviewport();

			uvl = 0; 
			uvr = 256 - 1; 
			uvb = 0; 
			uvt = VECY - 1;
			viewport(uvl, uvr, uvb, uvt);
			ortho2(-129, 129, -129, 129);
			getmatrix(uvmat);

			ytl = uvr + 1; 
			ytr = VECX - 1; 
			ytb = 0; 
			ytt = VECY - 1;
			viewport(ytl, ytr, ytb, ytt);
			ortho2(-1, Width + 1, -1, 257);
			getmatrix(ytmat);

			popviewport();
			popmatrix();

	/* 
     * Set the GL to lrectwrite from top to bottom
     * (the default is top to bottom). This way the
     * sense of the video frame is the same as that 
     * of the graphics window.
     */

	/* Set the packing type for the video board */
	if (packing != VL_PACKING_INVALID)
	{
		val.intVal = packing;
		if (vlSetControl(vlSvr, vlPath, drn, VL_PACKING, &val) < 0)
			vlPerror("Setting VL_PACKING");
	}

	/* Set the graphics to the same packing as the video board */
	vlGetControl(vlSvr, vlPath, drn, VL_PACKING, &val);
	packing = val.intVal;
	switch (val.intVal)
	{
	case VL_PACKING_Y_8_P:
		/*
	 * XXX -gordo
	 *  this needs to restore the colormap
	 */
		{
			int i;

			/* Use colormap mode for greyscale */
			cmode();
			for (i = 0; i < 256; i++)
				mapcolor(i, i, i, i);
			pixmode(PM_SIZE, 8);	/* Pixels are 8 bit Greyscale */
			gconfig();      /* Reconfigure graphics */
		}
		break;

	case VL_PACKING_RGB_332_P:
		pixmode(PM_SIZE, 9);	/* Pixels are 8 bit RGB */
		break;

	case VL_PACKING_RBG_323:
		pixmode(PM_SIZE, 8);
		break;

	case VL_PACKING_YVYU_422_8:
		pixmode(PM_SIZE, 16);
		break;

		/* Unknown packing type, use RGB as the default */
	default:
		packing = val.intVal = VL_PACKING_RGB_8;
		if (vlSetControl(vlSvr, vlPath, drn, VL_PACKING, &val) < 0)
			vlPerror("Setting RGB");
		vlGetControl(vlSvr, vlPath, drn, VL_PACKING, &val);
		Debug("packing type now is %s\n", packing_name(val.intVal));
		if (val.intVal != packing)
		{
			fprintf(stderr, "%s: Error, could not set Packing to %s", _progName,
			    packing_name (packing));
			exit(1);
		}
		/*
	     * There is no break here intentionally... the packing should
	     * now be VL_PACKING_RGB, and thus should fall through...
	     */
	case VL_PACKING_RGB_8:
	case VL_PACKING_RGBA_8:
		break;
	}

	vlGetControl(vlSvr, vlPath, drn, VL_SIZE, &val);
	if (xsize != val.xyVal.x || ysize != val.xyVal.y) {
		xsize = val.xyVal.x;
		ysize = val.xyVal.y;
		prefsize(xsize, ysize);
		winconstraints();
		reshapeviewport();
		Debug("after VL_PACKING size is now %dx%d\n", xsize, ysize);
	}

	/* Set up the ring buffer for data transfer */
	transferBuf = vlCreateBuffer(vlSvr, vlPath, drn, buffers);

	/* Associate the ring buffer with the path */
	vlRegisterBuffer(vlSvr, vlPath, drn, transferBuf);

	/*
     * Set ProcessEvent() as the callback for a transfer complete,
     * transfer failed and stream preempted events */
	vlAddCallback(vlSvr, vlPath, VLTransferCompleteMask|VLStreamPreempted
	    |VLTransferFailedMask, ProcessEvent, NULL);

	/* Tell the VL that the GL event handler will handle GL events */
	vlRegisterHandler(vlSvr, qgetfd(), (VLEventHandler)ProcessGlEvent,
	    (VLPendingFunc) qtest, (void *)win);

	/* Begin the data transfer */
	if (vlBeginTransfer(vlSvr, vlPath, 0, NULL) < 0)
	{
		vlPerror("vlBeginTransfer");
		exit(1);
	}

	/* Get the start time used in calculating frame rate */
	initStatistics();

	/* Main event loop */
	vlMainLoop();
}

/* Handle video library events (only transfer complete events in this example) */
void
ProcessEvent(VLServer svr, VLEvent *ev, void *data)
{
	int count;
	static int frameNum;
	static ulong lasttime = -1;
	static ulong lastsec;
	int timeDiff;
	DMediaInfo *dmInfo;
	VLInfoPtr info;
	char *dataPtr;

	/* Display the frame rate at a user specified interval */
	if (frameInterval > 0 && frameCount > frameInterval)
	{
		reportStatistics();
		frameCount = 0;
	}

	switch (ev->reason)
	{
	case VLTransferComplete:
		framenumber++;

		/* Get a pointer to the most recently captured frame */
		info = vlGetLatestValid(svr, transferBuf);
		if (!info)
			break;

		/* Get the valid video data from that frame */
		dataPtr = vlGetActiveRegion(svr, transferBuf, info);

		/* Time stamp the frame */
		if (time_on)
		{
			dmInfo = vlGetDMediaInfo(svr, transferBuf, info);
			if (lasttime != -1)
			{
				timeDiff = (dmInfo->time.tv_sec - lastsec) * TIME_GRANULARITY;
				timeDiff += (int)(dmInfo->time.tv_usec - lasttime + 500) /
				    (1000000 / TIME_GRANULARITY);
				if (timeDiff >= TIME_ARRAY_SIZE)
					timeDiff = TIME_ARRAY_SIZE-1;
				time_array[timeDiff] += 1;
			}
			lasttime = dmInfo->time.tv_usec;
			lastsec = dmInfo->time.tv_sec;
		}

		/* If the user didn't request fast capture write the frame to the screen */

/*		lrectwrite(0, 0, xsize-1, ysize-1, (ulong *)dataPtr);
		if (double_buffer)
			swapbuffers();
*/
		draw_vector((ushort*) dataPtr);

		/* Done with that frame, free the memory used by it */
		vlPutFree(svr, transferBuf);
		frameCount++;

		/* If we've gotten all the frames requested, quit */
		if (framenumber >= totalCount)
		{
			if (frameInterval > 0)
				reportStatistics();
			exitcapture();
		}
		break;

	case VLTransferFailed:
		/*exitcapture();*/
		break;

	case VLStreamPreempted:
		fprintf(stderr, "%s: Stream was preempted by another Program\n",
		    _progName);
		/*exitcapture();*/
		break;

	default:
		printf("Got Event %d\n", ev->reason);
		break;
	}
}

/* Handle graphics library events */
void
ProcessGlEvent(int fd, void *win)
{
	static short val;

	switch (qread(&val))
	{
		/* Toggle double buffer mode */
	case DKEY :
		if (val == 1) /* Get key downs only */
		{
			if (double_buffer = 1 - double_buffer)
			{
				doublebuffer();
				Debug("double buffer mode\n");
			}
			else 
			{
				singlebuffer();
				Debug("single buffer mode\n");
			}
			/* Set up the graphics subsystem */
			gconfig();
		}
		break;

		/* Quit */
	case QKEY :
	case ESCKEY :
		if (val != 1) /* Ignore key releases */
			break;

	case WINSHUT:
	case WINQUIT:
		exitcapture();
		break;
	}
}

void
exitcapture()
{
	int loop, j;


	/* End the data transfer */
	vlEndTransfer(vlSvr, vlPath);

	/* Disassociate the ring buffer from the path */
	vlDeregisterBuffer(vlSvr, vlPath, drn, transferBuf);

	/* Destroy the path, free the memory it used */
	vlDestroyPath(vlSvr,vlPath);

	/* Destroy the ring buffer, free the memory it used */
	vlDestroyBuffer(vlSvr, transferBuf);

	/* Disconnect from the daemon */
	vlCloseVideo(vlSvr);

	/* Print the time stamps from the captured frames */
	if (time_on)
		for (loop = 0; loop < TIME_ARRAY_SIZE; ) {
			printf("%3d: ", loop);
			for (j = 0; j < 5; j++)
				printf("\t%d", time_array[loop++]);
			printf("\n");
		}

	exit(0);
}

