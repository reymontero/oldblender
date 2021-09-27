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


/* uit netnews:

 *   email: cmaae47@ic.ac.uk (Thomas Sippel - Dau) (uk.ac.ic on Janet)
 *   voice: +44 (1)71 594 6904 (day), or +44 (1)71 594 6957 (fax)
 *   snail: Imperial College of Science, Technology and Medicine

cc -DR -O2 performance.c -lm -o performance
cc -DR -O2 -mips2 performance.c -lm -o performance

 25 MHz 486 under DOS                0.45   Mflops 
 25 MHz 486 under SCO Unix           0.95   Mflops
 66 MKz 486 DX2 under Linux and gcc  2.0    Mflops
 RS/6000 model 530   ( 25 MHz)       2.6    Mflops
 CDC 4340                            2.8    Mflops 
 Sparkstation 2                      3.2    Mflops 
 Iris 4D/30                          3.4    Mflops 
 Iris Indigo         ( 33 MHz)       3.7    Mflops
 alpha model 300L    (100 MHz)       6.3    Mflops
 HP 7000 model 720   ( 50 MHz)       6.5    Mflops
 Iris Indigo R4000   (100 MHz)       6.5    Mflops
 Iris Indigo R4000   (100 MHz)       8.9    Mflops (compiled with -mips2)

 BIJ NEOGEO:
 
 quad	(R4000 100MHz)				 8.99   Mflops (-mips2)
 indigo (R3000 33 MHz)				 3.85   Mflops
 iris   (R3000 30 MHz)				 3.56   Mflops

 */

/****   benchmark for testing floating point performance
 *
 *      fpr - Find Prime Root modulo a prime.
 *
 **     13. 7. 1987    original cast using Ansi C time interface   tsd
 *      30. 6. 1994    readied for distribution                    tsd
 *
 **     Copyright (C) July 1987 Thomas Sippel - Dau
 *
 *      This program may by copied and adapted for other systems
 *      as long as the following conditions are met:
 *
 *      o   this copyright notice must not be removed, altered or mutilated
 *      o   the program may only be used or adapted for peaceful purposes
 *      o   the program and adaptions must be made available in source form
 *          to anybody who is being offered a binary translation
 *      o   the weighting of 8 flop for the inner loop must be kept for
 *          results published
 */

/*      Useful primes for this program:
 *
 *      257, 8191, 65521, 65537, 999959, 999961, 999983, 9999943, 99999941
 *
 *      The formal worst case requirement for the algorithm is that the
 *      prime number (the first argument) is less than the square root of
 *      the maximum number the architecture can handle, i.e. for 32 bit
 *      integers, the prime number should be less than 65536 for unsigned
 *      and less than 32768 for signed ints. In practice, the biggest number
 *      that is actually generated is (limit-1)*(seed+n), for a small n.
 *      The asymptotic density of prime roots is about 1/e, so some should
 *      be found fairly rapidly.
 *
 *      If the limit is not a prime, this program is in an infinite loop.
 *      This is done to defeat compiler optimisations, it is NOT a bug.
 *
 *      The central loop can be abandoned when the local iteration count
 *      (cnt) is greater than (limit-1)/2, but a compiler that can do that
 *      optimisation from general principles has not yet come my way.
 */
#include  <stdio.h>
#include  <time.h>

/*      Some C compilers use CLK_TCK 1,000,000 for some bogus compatibility
 *      arguments. A correctly implemented ansi C compiler will skip the
 *      following code. In any case, cross-check results with the system
 *      processing time reporting to give reasonable results.
 */

#ifndef CLK_TCK
#define CLK_TCK 100    /* this may be 60 on some machines - best use ansi */

#include <sys/types.h>
#include <sys/times.h>

#define clock_t long

clock_t clock ()

{
    struct tms t;
    times ( &t );
    return ( t.tms_utime + t.tms_stime ); /* no children */
}

#endif  /* clock () for systems not yet bothering to support ansi C */

#ifdef R
#define reg register
#else
#define reg
#endif

 main ( argc, argv )
 int  argc;
 char **argv;

 {
    reg long int  prim, test, rest, quot, cnt, its;
             long int xprim = 999959, xtest =100;
             
    reg double prod;
    clock_t secs;

    if ( argc > 1 ) xprim = atoi ( argv [ 1 ] );
    if ( argc > 2 ) xtest = atoi ( argv [ 2 ] );
    if ( argc != 3 ) help ( *argv );
    
    secs = clock ();
    its = 0;
    prim = xprim;
    test = xtest % prim;
    if ( prim < 0L ) prim = -prim;
    if ( test < 0L ) test = -test;

    do { rest = 1;
         cnt  = 1;
         test++;

         do { prod = test * ( double ) rest;          /* 1 flop, 2 conv  */
              quot = prod / prim;                     /* 1 flop, 2 conv  */
              rest = prod - prim * ( double ) quot;   /* 2 flop, 3 conv  */
              cnt++;                                  /* 1 integer inc   */
              } while ( rest > 1L );                  /* test and jump   */
                                        /* counting conversion as half a *
                                         * floating point operation, we  *
                                         * should get about 8 flops/loop */

         its += cnt++;
         } while ( cnt < prim && prim > test );
    
    secs = clock () - secs;
    prod = secs / ( double ) (10000.0*CLK_TCK);
    printf ( "%9s: %d is a prime root of %d, found after %d tries\n",
            *argv, test, prim, its );
    printf ( "%9s: %.3f seconds prime root extraction time, %.3f kiloflops\n", 
            *argv, prod, 8 * its / ( 1000 * prod ) );
    exit (0);
}

help ( progname )
char *progname;

{
    fprintf ( stderr, "Syntax: %s <limit> <seed>\n", progname );
    fprintf ( stderr, "%s%s%s%s",
            "Function: find a prime root modulo <limit>\n",
            "Parameters:\n",
            "  <limit>  a prime number to be the modulus\n",
            "  <seed>   value past which to start searching\n" );   
}


