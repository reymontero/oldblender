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

An Algorithm for Automatically Fitting Digitized Curves
by Philip J. Schneider
from "Graphics Gems", Academic Press, 1990

ADAPTED: 5-93,  NeoGeo

*/

/* mogelijke verbeteringen:
 * 
 * - punten opnieuw parametriseren (constante afstand)
 * - interactief knippen
 * - handles beter berekenen.
 * 
 * - standaard een aantal keer itereren geeft betere resultaten.
 */

/*  fit_cubic.c	*/									
/*	Piecewise cubic fitting code	*/

#include <local/Trace.h>
#undef ABS
#undef SGN
#undef SWAP
#undef PI
#undef TRUE
#undef FALSE

#include "/usr/people/insane/public/gems1/GraphicsGems.h"
#include <stdio.h>
#include <math.h>

/* #include <sys/sysmips.h>	*/ /* voor fp-exeption trap */

typedef Point2 *BezierCurve;

/* Forward declarations */
void				FitCurve();
static	void		FitCubic();
static	double		*Reparameterize();
static	double		NewtonRaphsonRootFind();
static	Point2		Bezier();
static	double 		B0(), B1(), B2(), B3();
static	Vector2		ComputeLeftTangent();
static	Vector2		ComputeRightTangent();
static	Vector2		ComputeCenterTangent();
void				ComputeCenterTangents();
static	double		ComputeMaxError();
static	double		*ChordLengthParameterize();
static	BezierCurve	GenerateBezier();
static	Vector2		V2AddII();
static	Vector2		V2ScaleII();
static	Vector2		V2SubII();

#define MAXPOINTS	1000		/* The most points you can have */

#define SPLT_ERR	0	/* grootste error */
#define SPLT_MID	1	/* in het midden */
#define SPLT_EXT	2	/* op extremen */
#define SPLT_BND	3	/* op maximale buiging */

#define DFLT_ERROR 8.0

int splitmode = SPLT_EXT;
int optim = TRUE;
long segcount=0;
float *BezierData=0;


void AddBezierSeg(curve)
BezierCurve curve;
{
	float *fp;
	long i;
	
	fp= BezierData+ 3*segcount*2;
	
	for(i = 0; i < 4; i++, fp+=2) {
	
		fp[0] = curve[i].x;
		fp[1] = curve[i].y;
	}
	
	if(3*segcount<MAXPOINTS) segcount++;
	
}

/*
 *  main:
 *	(Dit is veranderd)
 
	aanroepen als:
	FitCurve(d, i, error, type, out);
		d: doubles
		i: aantal
		error: oppervlakte
		type: 0= Bezier,  1= BezTriples
		out: pointerpointer naar output
		
	er wordt een datablok gemaakt van curvesegmenten.
	deze worden ahv type omgezet en als pointer in out geschreven

*/


void FitCurve(d, nPts, error, type, out)
Point2	*d;			/*  Array of digitized points	*/
int		nPts;		/*  Number of digitized points	*/
double	error;		/*  User-defined error squared	*/
long type;
long **out;
{
    Vector2	tHat1, tHat2;	/*  Unit tangent vectors at endpoints */
	struct Bezier *bez;
	float *data, *fp;
	long a;
	
	
	/* globaal array */
	BezierData= callocN(MAXPOINTS*2*4+16, "FitCurve");

    tHat1 = ComputeLeftTangent(d, 0);
    tHat2 = ComputeRightTangent(d, nPts - 1);
    FitCubic(d, 0, nPts - 1, tHat1, tHat2, error);
	
	if(segcount) {
		if(type==0) {
			bez= callocN(sizeof(struct Bezier)+3*3*4*(segcount+10), "Fitcurve2");
			bez->cp= segcount+1;
			data= (float *)(bez+1);
			fp= BezierData;
			
			/* eerste handle */
			data[0]= 2*fp[0]-fp[2];
			data[1]= 2*fp[1]-fp[3];
			data+= 3;
			for(a=1; a<3*bez->cp; a++) {
				data[0]= fp[0];
				data[1]= fp[1];
				data+= 3;
				fp+= 2;
			}
			fp-= 2;
			data[0]= 2*fp[0]-fp[-2];
			data[1]= 2*fp[1]-fp[-1];
			
			*out= (long *)bez;
		}
	}
	else *out= 0;
	
	freeN(BezierData);
}



/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 */
 
static void FitCubic(d, first, last, tHat1, tHat2, error)
    Point2	*d;					/*  Array of digitized points */
    int		first, last;		/* Indices of first and last pts in region */
    Vector2	tHat1, tHat2;		/* Unit tangent vectors at endpoints */
    double	error;				/*  User-defined error squared	   */
{
    BezierCurve	bezCurve;		/*Control points of fitted Bezier curve*/
    double	*u;					/*  Parameter values for point  */
    double	*uPrime;			/*  Improved parameter values */
    double	maxError;			/*  Maximum fitting error	 */
    int		splitPoint;			/*  Point to split point set at	 */
    int		nPts;				/*  Number of points in subset  */
    double	iterationError;		/*Error below which you try iterating  */
    int		maxIterations = 4;	/*  Max times to try iterating  */
    Vector2	tHatCenter;   		/* Unit tangent vector at splitPoint */
	Vector2 tHatLeft,tHatRight;	/* om vector handles mogelijk te maken */
    int		i;

    iterationError = error * error;
    nPts = last - first + 1;

    /*  Use heuristic if region only has two points in it */
    if (nPts == 2) {
	    double dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;

		bezCurve = (Point2 *)mallocN(4 * sizeof(Point2), "FitCubic");
		bezCurve[0] = d[first];
		bezCurve[3] = d[last];
		V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
		V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
		AddBezierSeg(bezCurve);
		freeN(bezCurve);
		return;
    }

    /*  Parameterize points, and attempt to fit curve */
    u = ChordLengthParameterize(d, first, last);
    bezCurve = GenerateBezier(d, first, last, u, tHat1, tHat2);

    /*  Find max deviation of points to fitted curve */
    maxError = ComputeMaxError(d, first, last, bezCurve, u, &splitPoint);

	if (maxError < error) {
		AddBezierSeg(bezCurve);
		freeN(bezCurve);
		freeN(u);
		return;
    }
	
    /*  If error not too large, try some reparameterization  */
    /*  and iteration */
    if (maxError < iterationError) {
		for (i = 0; i < maxIterations; i++) {
	    	uPrime = Reparameterize(d, first, last, u, bezCurve);
			freeN(bezCurve);
	    	bezCurve = GenerateBezier(d, first, last, uPrime, tHat1, tHat2);
	    	maxError = ComputeMaxError(d, first, last, bezCurve, uPrime, &splitPoint);
	    	if (maxError < error) {
				AddBezierSeg(bezCurve);
				freeN(bezCurve);
				freeN(u);
				freeN(uPrime);
				return;
			}
			freeN((char *)u);
			u = uPrime;
		}
	}
	freeN(bezCurve);
	freeN(u);
	
    /* Fitting failed -- split at max error point and fit recursively */
	
	if (splitmode != SPLT_ERR) splitPoint = (first + last) / 2;
	if (splitmode == SPLT_EXT) {
		/* zoek naar extreem (x of y) in spline */
		/* degene het dichst bij het midden wint */
		/* misschien dat ook naar 45 gekeken kan worden */
		
		char * splitarr;	/* jes or no split */
		long i, c;
		long dist, mdist, best;
		float dx, dy;
		
		splitarr = (char *) mallocN(nPts, "FitCubic2");
		
		for (i = first; i < last; i ++){
			c = 0;
			dx = d[i].x - d[i + 1].x;
			dy = d[i].y - d[i + 1].y;
			if (dx < 0.0) c |= 1;
			else if (dx > 0.0) c |= 2;
			if (dy < 0.0) c |= 4;
			else if (dy > 0.0) c |= 8;
			/*if (fabsf(dx) < fabsf(dy)) c |= 16;*/
			splitarr[i - first] = c;
		}
		
		/* zoek naar omslagpunten */
		for (i = 0; i < nPts - 1; i++) splitarr[i] ^= splitarr[i + 1];
		
		best = splitPoint; mdist = 0xffffff;
		/* eventueel nog optimaliseren naar meerdere omslagen na elkaar ?? */
		for (i = 1; i < nPts - 3; i++) {
			if (splitarr[i]) {
				dist = (i + first + 1) - splitPoint;
				if (dist < 0) dist = -dist;
				if (dist < mdist) {
					best = i + first + 1;
					mdist = dist;
				}
			}
		}
		splitPoint = best;
		freeN(splitarr);
	}
	
/*
    tHatCenter = ComputeCenterTangent(d, splitPoint);
    FitCubic(d, first, splitPoint, tHat1, tHatCenter, error);
    V2Negate(&tHatCenter);
    FitCubic(d, splitPoint, last, tHatCenter, tHat2, error);
*/
    ComputeCenterTangents(d, splitPoint, &tHatLeft, &tHatRight);
    FitCubic(d, first, splitPoint, tHat1, tHatLeft, error);
    FitCubic(d, splitPoint, last, tHatRight, tHat2, error);
}


/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 *
 */
static BezierCurve  GenerateBezier(d, first, last, uPrime, tHat1, tHat2)
    Point2	*d;				/*  Array of digitized points	*/
    int		first, last;	/*  Indices defining region	*/
    double	*uPrime;		/*  Parameter values for region */
    Vector2	tHat1, tHat2;	/*  Unit tangents at endpoints	*/
{
    int 	i;
    Vector2 A[MAXPOINTS][2];		/* Precomputed rhs for eqn	*/
    int 	nPts;					/* Number of pts in sub-curve */
    double 	C[2][2];				/* Matrix C		*/
    double 	X[2];					/* Matrix X			*/
    double 	det_C0_C1,				/* Determinants of matrices	*/
    	   	det_C0_X,
	   		det_X_C1;
    double 	alpha_l,				/* Alpha values, left and right	*/
    	   	alpha_r;
    Vector2 	tmp;				/* Utility variable		*/
    BezierCurve	bezCurve;			/* RETURN bezier curve ctl pts	*/

    bezCurve = (Point2 *)mallocN(4 * sizeof(Point2), "Generate Bezier");
    nPts = last - first + 1;

 
    /* Compute the A's	*/
    for (i = 0; i < nPts; i++) {
		Vector2		v1, v2;

		v1 = tHat1; v2 = tHat2;
		V2Scale(&v1, B1(uPrime[i]));
		V2Scale(&v2, B2(uPrime[i]));
		A[i][0] = v1;
		A[i][1] = v2;
    }

    /* Create the C and X matrices	*/
    C[0][0] = C[0][1] = C[1][0] = C[1][1] = 0.0;
    X[0]    = X[1]    = 0.0;

    for (i = 0; i < nPts; i++) {
        C[0][0] += V2Dot(&A[i][0], &A[i][0]);
		C[1][0] = C[0][1] += V2Dot(&A[i][0], &A[i][1]);
		C[1][1] += V2Dot(&A[i][1], &A[i][1]);

		tmp = V2SubII(d[first + i],
	        V2AddII(
	          V2ScaleII(d[first], B0(uPrime[i])),
		    	V2AddII(
		      		V2ScaleII(d[first], B1(uPrime[i])),
		        			V2AddII(
	                  		V2ScaleII(d[last], B2(uPrime[i])),
	                    		V2ScaleII(d[last], B3(uPrime[i]))))));
	

		X[0] += V2Dot(&A[i][0], &tmp);
		X[1] += V2Dot(&A[i][1], &tmp);
    }
	
    /* Compute the determinants of C and X	*/
    det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
    det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
    det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

    /* Finally, derive alpha values	*/
    if (det_C0_C1 == 0.0) {
		det_C0_C1 = (C[0][0] * C[1][1]) * 10e-12;
    }
	
	/*if (det_C0_C1) {*/
		alpha_l = det_X_C1 / det_C0_C1;
		alpha_r = det_C0_X / det_C0_C1;
	/*} else alpha_l = -1.0;*/

    /*  If alpha negative, use the Wu/Barsky heuristic (see text) */
    if (alpha_l < 0.0 || alpha_r < 0.0) {
		double	dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;

		bezCurve[0] = d[first];
		bezCurve[3] = d[last];
		V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
		V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
		return (bezCurve);
    }

    /*  First and last control points of the Bezier curve are */
    /*  positioned exactly at the first and last data points */
    /*  Control points 1 and 2 are positioned an alpha distance out */
    /*  on the tangent vectors, left and right, respectively */
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    V2Add(&bezCurve[0], V2Scale(&tHat1, alpha_l), &bezCurve[1]);
    V2Add(&bezCurve[3], V2Scale(&tHat2, alpha_r), &bezCurve[2]);
    return (bezCurve);
}


/*
 *  Reparameterize:
 *	Given set of points and their parameterization, try to find
 *   a better parameterization.
 *
 */
static double *Reparameterize(d, first, last, u, bezCurve)
    Point2	*d;				/*  Array of digitized points	*/
    int		first, last;	/*  Indices defining region	*/
    double	*u;				/*  Current parameter values	*/
    BezierCurve	bezCurve;	/*  Current fitted curve	*/
{
    int 	nPts = last-first+1;	
    int 	i;
    double	*uPrime;		/*  New parameter values	*/

    uPrime = (double *)mallocN(nPts * sizeof(double), "Reparametr");
    for (i = first; i <= last; i++) {
		uPrime[i-first] = NewtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
    }
    return (uPrime);
}



/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 */
static double NewtonRaphsonRootFind(Q, P, u)
    BezierCurve	Q;			/*  Current fitted curve	*/
    Point2 		P;		/*  Digitized point		*/
    double 		u;		/*  Parameter value for "P"	*/
{
    double 		numerator, denominator;
    Point2 		Q1[3], Q2[2];	/*  Q' and Q''			*/
    Point2		Q_u, Q1_u, Q2_u; /*u evaluated at Q, Q', & Q''	*/
    double 		uPrime;		/*  Improved u			*/
    int 		i;
    
    /* Compute Q(u)	*/
    Q_u = Bezier(3, Q, u);
    
    /* Generate control vertices for Q'	*/
    for (i = 0; i <= 2; i++) {
		Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
		Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
    }
    
    /* Generate control vertices for Q'' */
    for (i = 0; i <= 1; i++) {
		Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
		Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
    }
    
    /* Compute Q'(u) and Q''(u)	*/
    Q1_u = Bezier(2, Q1, u);
    Q2_u = Bezier(1, Q2, u);
    
    /* Compute f(u)/f'(u) */
    numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
    denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		      	  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
    
    /* u = u - f(u)/f'(u) */
    uPrime = u - (numerator/denominator);
    return (uPrime);
}

	
		       
/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 * 
 */
static Point2 Bezier(degree, V, t)
    int		degree;		/* The degree of the bezier curve	*/
    Point2 	*V;		/* Array of control points		*/
    double 	t;		/* Parametric value to find point for	*/
{
    int 	i, j;		
    Point2 	Q;	        /* Point on curve at parameter t	*/
    Point2 	*Vtemp;		/* Local copy of control points		*/

    /* Copy array	*/
    Vtemp = (Point2 *)mallocN( (unsigned)((degree+1)* sizeof (Point2)), "Bezier(");
    for (i = 0; i <= degree; i++) {
		Vtemp[i] = V[i];
    }

    /* Triangle computation	*/
    for (i = 1; i <= degree; i++) {	
		for (j = 0; j <= degree-i; j++) {
	    	Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
	    	Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
		}
    }

    Q = Vtemp[0];
    freeN((char *)Vtemp);
    return Q;
}


/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static double B0(u)
    double	u;
{
    double tmp = 1.0 - u;
    return (tmp * tmp * tmp);
}


static double B1(u)
    double	u;
{
    double tmp = 1.0 - u;
    return (3 * u * (tmp * tmp));
}

static double B2(u)
    double	u;
{
    double tmp = 1.0 - u;
    return (3 * u * u * tmp);
}

static double B3(u)
    double	u;
{
    return (u * u * u);
}



/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 *Approximate unit tangents at endpoints and "center" of digitized curve
 */
 
static Vector2 ComputeLeftTangent(d, end)
    Point2	*d;			/*  Digitized points*/
    int		end;		/*  Index to "left" end of region */
{
    Vector2	tHat1;
    tHat1 = V2SubII(d[end+1], d[end]);
    tHat1 = *V2Normalize(&tHat1);
    return tHat1;
}

static Vector2 ComputeRightTangent(d, end)
    Point2	*d;			/*  Digitized points		*/
    int		end;		/*  Index to "right" end of region */
{
    Vector2	tHat2;
    tHat2 = V2SubII(d[end-1], d[end]);
    tHat2 = *V2Normalize(&tHat2);
    return tHat2;
}


static Vector2 ComputeCenterTangent_oud(d, center)
    Point2	*d;			/*  Digitized points			*/
    int		center;		/*  Index to point inside region	*/
{
    Vector2	V1, V2, tHatCenter;

    V1 = V2SubII(d[center-1], d[center]);
    V2 = V2SubII(d[center], d[center+1]);
	V2Normalize(&V1); V2Normalize(&V2);
	
    tHatCenter.x = V1.x + V2.x;
    tHatCenter.y = V1.y + V2.y;
    tHatCenter = *V2Normalize(&tHatCenter);
    return tHatCenter;
}


static Vector2 ComputeCenterTangent(d, center)
    Point2	*d;			/*  Digitized points			*/
    int		center;		/*  Index to point inside region	*/
{
    Vector2	tHatCenter;

    tHatCenter = V2SubII(d[center-1], d[center + 1]);
 	V2Normalize(&tHatCenter);
	
    return tHatCenter;
}


void ComputeCenterTangents(d, center, V1, V2)
    Point2	*d;			/*  Digitized points			*/
    int		center;		/*  Index to point inside region	*/
	Vector2 *V1, *V2;
{
    Vector2	tHatCenter;

    V2Sub(&d[center-1], &d[center], V1);
    V2Sub(&d[center+1], &d[center], V2);
	V2Normalize(V1); V2Normalize(V2);
	if (V2Dot(V1, V2) > -0.7) return;
	
	*V1 = *V2 = ComputeCenterTangent(d, center);
	V2Negate(V2);
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 */
static double *ChordLengthParameterize(d, first, last)
    Point2	*d;			/* Array of digitized points */
    int		first, last;		/*  Indices defining region	*/
{
    int		i;	
    double	*u;			/*  Parameterization		*/

    u = (double *)mallocN((unsigned)(last-first+1) * sizeof(double), "ChordLength");

    u[0] = 0.0;
    for (i = first+1; i <= last; i++) {
		u[i-first] = u[i-first-1] +
	  			V2DistanceBetween2Points(&d[i], &d[i-1]);
    }

    for (i = first + 1; i <= last; i++) {
		u[i-first] = u[i-first] / u[last-first];
    }

    return(u);
}




/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
*/
static double ComputeMaxError(d, first, last, bezCurve, u, splitPoint)
    Point2	*d;				/*  Array of digitized points	*/
    int		first, last;	/*  Indices defining region	*/
    BezierCurve	bezCurve;	/*  Fitted Bezier curve		*/
    double	*u;				/*  Parameterization of points	*/
    int		*splitPoint;	/*  Point of maximum error	*/
{
    int		i, j;
    double	maxDist;				/*  Maximum error		*/
    double	dist;					/*  Current error		*/
    Point2	P;						/*  Point on curve		*/
    Vector2	v;						/*  Vector from point to curve	*/

    *splitPoint = (last - first + 1)/2;
    maxDist = 0.0;
	
	/* sysmips(MIPS_FPSIGINTR, 0); */

    for (i = first + 1; i < last; i++) {				
		P = Bezier(3, bezCurve, u[i-first]);
		v = V2SubII(P, d[i]);
		dist = V2SquaredLength(&v);
		

		if (dist >= maxDist) {
	    	maxDist = dist;
	    	*splitPoint = i;
		}
    }
	
/* 	sysmips(MIPS_FPSIGINTR, 1); */

    return (maxDist);
}

static Vector2 V2AddII(a, b)
    Vector2 a, b;
{
    Vector2	c;
    c.x = a.x + b.x;  c.y = a.y + b.y;
    return (c);
}
static Vector2 V2ScaleII(v, s)
    Vector2	v;
    double	s;
{
    Vector2 result;
    result.x = v.x * s; result.y = v.y * s;
    return (result);
}

static Vector2 V2SubII(a, b)
    Vector2	a, b;
{
    Vector2	c;
    c.x = a.x - b.x; c.y = a.y - b.y;
    return (c);
}

