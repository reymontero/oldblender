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

#include <sys/types.h>

#include <local/iff.h>
#include <local/util.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <malloc.h>	/* mallopt */
#include <movie.h>
#include <cdaudio.h>
#include <dmedia/cl.h>
#include <dmedia/cl_cosmo.h>

/*

zipfork "make movie "                                                     
zipfork "movie -l -q 99 /render/second.mv /render/curry/anim/04[012]? >/dev/console"                                       
zipfork "movie -l /render/third.mv /render/curry/anim/04[012]? >/dev/console"                                       

*/

/*
 * Doen:
 * 
 * Movie in vieren splitsen:
 * 
 * 1. lezer
 * 2. bewerker
 * 3. comprimeerder
 * 4. schrijver
 * 
 * Plaatjes kunnen met behulp van een enkel adres doorgegeven worden.
 * Het ene proces zet ze, het volgende proces zet ze op nul.
 */

extern rectcpy();
extern rectfill();

/* Global Variables */

CL_Handle	compr, soft_compr;
int			compr_params[64];
int			numfields = 2, bufsize, quality = 0, qualnow, qualindex, inputthread;
char		*comp_buf;
long		index;


int			follow = TRUE, alpha = FALSE, anti = FALSE, dither = FALSE, use_size = FALSE, xfilt = 0, yfilt = 0;
int			inx, iny, outx, outy;
double		gamval = 1.0;
ImBuf		* nextibuf = 0;
int			more_pics = TRUE;

int Argc;
char ** Argv;

void quit(char * error) {
	
	perror(error);
	exit(1);
}

void de_interlace_swap(ImBuf *ibuf)
{
	ImBuf * tbuf1, * tbuf2;
	
	if (ibuf == 0) return;
	if (ibuf->flags & IB_fields) return;
	ibuf->flags |= IB_fields;
	
	if (ibuf->rect) {
		/* kopieen aanmaken */
		tbuf1 = allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect, 0);
		tbuf2 = allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect, 0);
		
		ibuf->x *= 2;	
		rectop(tbuf1, ibuf, 0, 0, 0, 0, 32767, 32767, rectcpy);
		rectop(tbuf2, ibuf, 0, 0, tbuf2->x, 0, 32767, 32767, rectcpy);
	
		ibuf->x /= 2;
		rectop(ibuf, tbuf2, 0, 0, 0, 0, 32767, 32767, rectcpy);
		rectop(ibuf, tbuf1, 0, tbuf1->y, 0, 0, 32767, 32767, rectcpy);
		
		freeImBuf(tbuf1);
		freeImBuf(tbuf2);
	}
	ibuf->y /= 2;
}

void vert_dither(ImBuf *ibuf)
{
	int x, y, z;
	uchar *rect;
	
	for (y = 0; y < ibuf->y; y++) {
		rect = (uchar *) (ibuf->rect + (y * ibuf->x));
		for (x = ibuf->x / 2; x > 0; x--) {
			if (rect[0] != 0) rect[0]--;
			if (rect[1] != 0) rect[1]--;
			if (rect[2] != 0) rect[2]--;
			if (rect[3] != 0) rect[3]--;

			if (rect[4] != 255) rect[4]++;
			if (rect[5] != 255) rect[5]++;
			if (rect[6] != 255) rect[6]++;
			if (rect[7] != 255) rect[7]++;
			
			rect += 8;
		}
	}
}


int loader() {
	int i, j, ofsx, ofsy;
	ImBuf * ibuf, * tbuf;
	char tmpname[1024];
	uchar * rect;
	
	for (i = 2; i < Argc; i++) {
		strcpy(tmpname, Argv[i]);
		
		do {
			ibuf = loadiffname(tmpname, IB_rect | IB_ttob);
			newname(tmpname, +1);
			
			if (ibuf) {
				if (alpha) {
					rect = (uchar *) ibuf->rect;
					for (j = ibuf->x * ibuf->y; j > 0; j--){
						rect[1] = rect[2] = rect[3] = rect[0];
						rect += 4;
					}
				}

				if (gamval != 1.0) gamwarp(ibuf, gamval);
				if (anti) antialias(ibuf);
				
				for (j = xfilt; j > 0; j--) filterx(ibuf);
				for (j = yfilt; j > 0; j--) filtery(ibuf);
				
				if (use_size) {
					if (numfields == 2 && iny != outy) scalefieldImBuf(ibuf, outx, outy);
					else scaleImBuf(ibuf, outx, outy);
				}
				
				
				for (j = dither; j > 0; j--) vert_dither(ibuf);
				
				if (ibuf->x != outx || ibuf->y != outy) {
					tbuf = allocImBuf(outx, outy, 32, IB_rect, 0);
					rectoptot(tbuf, 0, rectfill, 0x00);
					
					ofsx = (tbuf->x - ibuf->x) / 2;
					ofsy = (tbuf->y - ibuf->y) / 2;
					if (numfields == 2) ofsy &= ~1;
					
					rectop(tbuf, ibuf, ofsx, ofsy, 0, 0, 32767, 32767, rectcpy);
					freeImBuf(ibuf);
					strcpy(tbuf->name, ibuf->name);
					ibuf = tbuf;
				}
				
				if (numfields == 2) {
					rectop(ibuf, ibuf, 0, 0, 0, 1, 32767, 32767, rectcpy);
					de_interlace(ibuf);
				}
				
				/* ibuf doorgeven */

				while (nextibuf) sginap(1);
				nextibuf = ibuf;
			}

		}while(ibuf && follow);
	}
	
	more_pics = FALSE;
	exit(0);
}


int my_Compress(ulong * rect)
{
	int error = 0;
	
	compr_params[qualindex] = qualnow;
	clSetParams(compr, compr_params, index);

	while (clCompress(compr, numfields, rect, &bufsize, comp_buf) != numfields) {
		if (compr == soft_compr) quit("clCompress (software)");
		
		/* hardware opnieuw initialiseren */
		clCloseCompressor(compr);
		clOpenCompressor(CL_JPEG_COSMO, &compr);

		qualnow--;
		compr_params[qualindex] = qualnow;
		clSetParams(compr, compr_params, index);
		printf("retrying at quality %d\n", qualnow);
		error = TRUE;
	}
	
	return (error);
}

#define DIR_UP 1
#define DIR_DOWN 2
#define DIR_BOTH (DIR_UP | DIR_DOWN)

#define MAXQUAL quality
#define MINQUAL 30
#define FIRSTQUALSTEP 4

movie(int argc, char ** argv)
{
    DMparams	*movie_params, *image_params, *sound_params;
    MVid		movie, image, sound, optim; 
	ImBuf		*ibuf;
	long		i, cur_image;
	int			hardware_jpeg = FALSE, error, loop = MV_LOOP_CONTINUOUSLY;
	int			rate = 0, qualstep, lastqual, direction, xsizeset = FALSE, ysizeset = FALSE;
	char		tmpname[1024], cmd[1024];
	
	setbell(1);
	
	/* scaling / cropping auto maken;
	 * 
	 * - als formaatverschil +/- 5% is : croppen anders scalen
	 * - Het moet mogelijk zijn om de auto verdubbelaar uit te zetten
	 * zodat je het formaat van twee movies kunt matchen.
	 * - het moet eindelijk mogelijk zijn om movies zonder fields te
	 * kunnen maken / afspelen
	 * 
	 */
	
	while (argc > 1) {
		if (argv[1][0] != '-') break;
		i = 1;
		while (argv[1][i]) {
			switch(argv[1][i]) {
			case 'a':
				anti = TRUE;
				break;
			case 'c':
				use_size = FALSE;
				break;
			case 'd':
				dither ++;
				break;
			case 'o':
				loop = MV_LOOP_NONE;
				break;
			case 'k':
				alpha = TRUE;
				break;
			case 'p':
				loop = MV_LOOP_SWINGING;
				break;
			case 'q':
				if (argv[1][i + 1]) quality = atoi(argv[1] + i + 1);
				else {
					quality = atoi(argv[2]);
					argc--;
					argv++;
				}
				if (quality < 10) quality = 10;
				if (quality > 100) quality = 100;
				i = strlen(argv[1]) - 1;
				break;
			case 'r':
				if (argv[1][i + 1]) rate = atoi(argv[1] + i + 1);
				else {
					rate = atoi(argv[2]);
					argc--;
					argv++;
				}
				if (rate < 10) rate = 10;
				if (rate > 500) rate = 500;
				i = strlen(argv[1]) - 1;
				break;
			case '1':
				follow = FALSE;
				break;
			case 'g':
				if (argv[1][i + 1]) gamval = atof(argv[1] + i + 1);
				else {
					gamval = atof(argv[2]);
					argc--;
					argv++;
				}
				i = strlen(argv[1]) - 1;
				break;
			case 's':
				use_size = TRUE;
				break;
			case 'h':
				hardware_jpeg = TRUE;
				break;
			case 'x':
				xfilt++;
				break;
			case 'y':
				yfilt++;
				break;
			case '7':
			case '3':
				xsizeset = atoi(argv[1] + i);
				i = strlen(argv[1]) - 1;
				break;
			case '5':
			case '2':
				ysizeset = atoi(argv[1] + i);
				i = strlen(argv[1]) - 1;
				break;
			default:
				printf("unknown option %c\n", argv[1][i]);
				argc = 0;
			}
			i++;
		}
		argc--;
		argv++;
	}
	
	if (argc < 3) {
		printf("usage: %s [-o(nce) -p(ingpong)] [-q(uality) NUM] [-r(ate) NUM] [-s(cale)]\n[-y(filter)] [-x(filter)] [-a(nti-alias)] [-k(eymatte)] [-g(amma) FLOAT]\n[-1(image/argument)] [-720|768|360|384] [-576|288] [-c(rop)] [-d(ither)] \n[-h(ardware compression)] outmovie inpic1 [inpic2 ...] \n", argv[0]);
		ringbell();
		exit(0);
	}
	
	
	ibuf = loadiffname(argv[2], IB_test);
	if (ibuf == 0) quit(argv[2]);
	
	inx = ibuf->x;
	iny = ibuf->y;
	
	if (xsizeset) inx = xsizeset;
	if (ysizeset) iny = ysizeset;
	
	switch (inx) {
		case 360:
		case 384:
		case 720:
		case 768:
			outx = inx;
			break;
		default:
			if (inx <= 360) outx = 360;
			else if (inx <= 384) outx = 384;
			else if (inx <= 720) outx = 720;
			else outx = 768;
	}
	
	if (iny <= 288) {
		numfields = 1;
		outy = 288;
	} else {
		numfields = 2;
		outy = 576;
	}

	if (quality == 0) {
		if (rate) quality = 95;
		else quality = 90;
	}


	printf("\n  Movie settings:\n");
	printf("    input size : %4d x %3d\n", inx, iny);
	printf("    output size: %4d x %3d\n", outx, outy);
	{
		char * data[] = {"?", "non-", ""};
		printf("    generating %sinterlaced movie\n", data[numfields]);
	}
	if (rate) {
		printf("    using fixed rate: %d, with maximum quality: %d\n", rate, quality);
	} else {
		printf("    using fixed quality: %d\n", quality);
	}

	printf("\n  Image settings:\n");
	{
		char * data[] = {"cropping", "scaling"};
		printf("    using %s to fit image\n", data[use_size]);
	}
	{
		char * data[] = {"no", "1x", "2x", "3x", "4x"};
		printf("    %s x-filtering\n", data[xfilt]);
		printf("    %s y-filtering\n", data[yfilt]);
	}
	{
		char * data[] = {"no", "adding"};
		printf("    %s antialiasing\n", data[anti]);
	}

	printf("\n");
	
	/* initialiseren van de compressor */
	
	if (clOpenCompressor(CL_JPEG_SOFTWARE, &soft_compr) != SUCCESS) quit("clOpenCompressor");
	
	if (hardware_jpeg) {
		if (clOpenCompressor(CL_JPEG_COSMO, &compr) != SUCCESS) {
			ringbell();
			printf("can't use hardware compression\n");
			fflush(stdout);
			exit(1);
		}
		printf("using hardware compression\n");
	} else {
		compr = soft_compr;
		printf("using software compression\n");
	}
	
	printf("\n\n\n");
	
	index = 0;
	
	compr_params[index++]= CL_IMAGE_WIDTH;
	compr_params[index++]= outx;

	compr_params[index++]= CL_IMAGE_HEIGHT;
	compr_params[index++]= 288;
	
	compr_params[index++]= CL_JPEG_QUALITY_FACTOR;
	qualindex = index;
	compr_params[index++]= quality;

	compr_params[index++]= CL_ORIGINAL_FORMAT;
	compr_params[index++]= CL_RGBX;

	compr_params[index++]= CL_ORIENTATION;
	compr_params[index++]= CL_TOP_DOWN;

	compr_params[index++]= CL_INTERNAL_FORMAT;
	compr_params[index++]= CL_YUV422;

	/* this parameter must be set for non-queueing mode */
	compr_params[index++]= CL_ENABLE_IMAGEINFO;
	compr_params[index++]= 1;

	/* enable stream headers */
	compr_params[index++]= CL_STREAM_HEADERS;
	compr_params[index++]= TRUE;

	clSetParams(compr, compr_params, index);
	if (compr != soft_compr) clSetParams(soft_compr, compr_params, index);
	
	bufsize = 2 * clGetParam(compr, CL_COMPRESSED_BUFFER_SIZE);
	comp_buf = mallocN(bufsize, "cosmo_buffer");
	
	/* initialiseren van de movie */
		
    if (dmParamsCreate(&movie_params) != DM_SUCCESS) quit("dmParamsCreate");	
    if (dmParamsCreate(&image_params) != DM_SUCCESS) quit("dmParamsCreate");
		
    if (mvSetMovieDefaults(movie_params, MV_FORMAT_SGI_3) != DM_SUCCESS) quit("mvSetMovieDefaults");
	if (dmSetImageDefaults(image_params, outx, outy, DM_PACKING_RGBX) != DM_SUCCESS) quit("dmSetImageDefaults");
	
	/*
	 defaults are 
	 DM_MEDIUM = DM_IMAGE,
     DM_IMAGE_WIDTH = width, DM_IMAGE_HEIGHT = height, DM_IMAGE_RATE = 15.0,
     DM_IMAGE_COMPRESSION = DM_IMAGE_UNCOMPRESSED, DM_IMAGE_INTERLACING = DM_IMAGE_NONINTERLEAVED,
	 DM_IMAGE_PACKING = packing, DM_IMAGE_ORIENTATION = DM_BOTTOM_TO_TOP.
	*/
	
	dmParamsSetFloat(image_params, DM_IMAGE_RATE, 25.0);
	
	if (numfields == 2) dmParamsSetEnum(image_params, DM_IMAGE_INTERLACING, DM_IMAGE_INTERLACED_EVEN);
	else dmParamsSetEnum(image_params, DM_IMAGE_INTERLACING, DM_IMAGE_NONINTERLACED);
	
	dmParamsSetEnum(image_params, DM_IMAGE_ORIENTATION, DM_TOP_TO_BOTTOM);
	dmParamsSetString(image_params, DM_IMAGE_COMPRESSION, DM_IMAGE_JPEG);

    if (mvCreateFile(argv[1], movie_params, NULL, &movie) != DM_SUCCESS) quit("mvCreateFile");
	if (mvAddTrack(movie, DM_IMAGE, image_params, NULL, &image)) quit("mvAddTrack");
	if (mvSetLoopMode(movie, loop) != DM_SUCCESS) quit("mvSetMovieDefaults");

	cur_image = 0;
	qualnow = quality;
	rate *= 1024;
	
	Argc = argc;
	Argv = argv;
	
	if ((inputthread = sproc ((void (*)(void *))loader, PR_SALL)) == -1) {
		fprintf (stderr, "Unable to sproc to handle input\n");
		perror ("sproc");
		exit(0);
	}
	
	do {
		if (ibuf) freeImBuf(ibuf);

		while (nextibuf == 0 && more_pics) sginap(1);
		
		if (nextibuf) {
			ibuf = nextibuf;
			nextibuf = 0;
			
			if (rate == 0) {
				/* fallback in quality */
				qualnow = quality;
				my_Compress(ibuf->rect);
			} else {
				qualstep = FIRSTQUALSTEP;
				direction = 0;
				
				do {
					if (qualnow > MAXQUAL) qualnow = MAXQUAL;
					if (qualnow < MINQUAL) qualnow = MINQUAL;
	
					compr_params[qualindex] = qualnow;
					clSetParams(compr, compr_params, index);
	
					lastqual = qualnow;
					error = my_Compress(ibuf->rect);
					
					printf(" tried quality: %d, size %d\n", qualnow, bufsize);
					
					if (bufsize < 0.9 * rate) {
						if (error) {
							/* forget about this frame, retry next frame at old quality settting */
							qualnow = lastqual;
							break;
						}
						if (qualnow == MAXQUAL) break;
						direction |= DIR_UP;
						if (direction == DIR_BOTH) qualstep /= 2;
						qualnow += qualstep;
					} else if (bufsize > 1.1 * rate) {
						if (qualnow == MINQUAL) break;
						direction |= DIR_DOWN;
						if (direction == DIR_BOTH) qualstep /= 2;
						qualnow -= qualstep;
					} else break;
											
					if (qualstep == 0) {
						/* this was the last iteration. Make sure that the buffer isn't to big */
						if (bufsize < 1.1 * rate) break;
						else qualnow--;
					}
				} while (1);
				
				printf("used quality: %d\n", qualnow);
				
				if (bufsize < rate) qualnow++;
				else qualnow--;
				
			}
			
			if (mvInsertCompressedImage(image, cur_image, bufsize, comp_buf) != DM_SUCCESS) quit("mvInsertCompressedImage");
			printf("added frame %3d: length %6d: %s\n", cur_image, bufsize, ibuf->name);
			cur_image++;
		}
	} while(more_pics || nextibuf);

	wait(0);
	
	clCloseCompressor(compr);
	if (compr != soft_compr) clCloseCompressor(soft_compr);
	
    if (mvWrite(movie) != DM_SUCCESS) quit("mvWrite");
	if (mvClose(movie) != DM_SUCCESS) quit("mvClose");
	chmod(argv[1], 0664);

	
	/* optimize */
	
	printf("Optimizing\nPlease wait\n");
	
	sprintf(tmpname, "%s.tmp", argv[1]);
	
    if (mvCreateFile(tmpname, movie_params, NULL, &optim) != DM_SUCCESS) quit("mvCreateFile");
    if (mvOpenFile(argv[1], O_RDONLY, &movie) != DM_SUCCESS) quit("mvOpenFile");

	chmod(tmpname, 0666);

    if (mvOptimize(movie, optim) == DM_SUCCESS) {
		printf("Optimize succesfull\n");
		if (mvWrite(optim) == DM_SUCCESS) {
			if (mvClose(movie) != DM_SUCCESS) quit("mvClose");
			if (mvClose(optim) != DM_SUCCESS) quit("mvClose");
			
			sprintf(cmd, "mv -f %s %s", tmpname, argv[1]);
			if (system(cmd) < 0) {
				printf("Can't change %s. File saved as %s\n", argv[1], tmpname);
			}
			
			exit(0);
		}
	}
	/*printf("Couldn't optimize %s\n%s\n", argv[1], mvGetErrorStr(mvGetErrno()));*/
	if (mvClose(movie) != DM_SUCCESS) quit("mvClose");
	if (mvClose(optim) != DM_SUCCESS) quit("mvClose");
	remove(tmpname);
}


int mvcopy(char * srcname, char * dstname, int start, int end)
{
    DMparams	*movie_params, *image_params = 0, *audio_params = 0;
    MVid		movie, optim, dst_image, src_image, dst_audio, src_audio; 
	char		tmpname[1024];
	int			srcframe, dstframe;
	
	if (mvOpenFile(srcname, O_RDONLY, &movie) != DM_SUCCESS) quit("mvOpenFile");
	
	movie_params = mvGetParams(movie);

	if (mvCreateFile(dstname, movie_params, NULL, &optim) != DM_SUCCESS) quit("mvCreateFile");
	chmod(dstname, 0666);

	if (mvFindTrackByMedium(movie, DM_IMAGE, &src_image) == DM_SUCCESS) {
		image_params = mvGetParams(src_image);
		if (mvAddTrack(optim, DM_IMAGE, image_params, NULL, &dst_image)) quit("mvAddTrack");
	}

	if (mvFindTrackByMedium(movie, DM_AUDIO, &src_audio) == DM_SUCCESS) {
		audio_params = mvGetParams(src_audio);
		if (mvAddTrack(optim, DM_AUDIO, audio_params, NULL, &dst_audio)) quit("mvAddTrack");
	}
		
	dstframe = 0;
	
	for (srcframe = start; srcframe <= end; srcframe++, dstframe++) {
		if (image_params) {
			if (mvCopyFramesAtTime(src_image, srcframe, 1, 25, dst_image, dstframe, 25, DM_FALSE) == DM_FAILURE) break;
		}
		
		if (audio_params) {
			if (mvCopyFramesAtTime(src_audio, srcframe, 1, 25, dst_audio, dstframe, 25, DM_FALSE) == DM_FAILURE) break;
		}
		
		printf("  %d \r", dstframe + 1);
		fflush(stdout);
	}
		
	printf("done\n\n", srcframe);
	fflush(stdout);
	
	if (mvClose(optim) != DM_SUCCESS) quit("mvClose optim");
	if (mvClose(movie) != DM_SUCCESS) quit("mvClose movie");
}

mvcrop(int argc, char ** argv)
{
	char		tmpname[1024];
	int			start, end;
	
	if (argc < 4) {
		printf("usage: %s first last inmovie.mv [outmovie.mv]\n", argv[0]);
		printf("  If outmovie is not specified, inmovie.mv.crop is used.\n");
		printf("  Both first AND last image are copied to outmovie.mv.\n");
		printf("  Counting starts at frame # 1.\n");
		ringbell();
		exit(0);
	}
	
	start = atoi(argv[1]);
	end = atoi(argv[2]);
	
	start--;
	end--;
	
	if (start < 0) start = 0;
	
	if (argc == 5) strcpy(tmpname, argv[4]);
	else sprintf(tmpname, "%s.crop", argv[3]);
	
	mvcopy(argv[3], tmpname, start, end);	
}

mvoptimize(int argc, char ** argv)
{
	char		tmpname[1024];
	
	if (argc < 2) {
		printf("usage: %s inmovie.mv\n", argv[0]);
		ringbell();
		exit(0);
	}
	
	sprintf(tmpname, "%s.opt", argv[1]);
	
	mvcopy(argv[1], tmpname, 0, 9999999);
}

main(int argc, char ** argv)
{
	if (strcmp(argv[0], "mvcrop") == 0) mvcrop(argc, argv);
	else if (strcmp(argv[0], "mvoptimize") == 0) mvoptimize(argc, argv);
	else movie(argc, argv);
	
}

