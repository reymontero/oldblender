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


#include <stdio.h>

/* NIEUWE QSORT */

int (*vergfunc)();
int vergsize, temppivot[40];


int partitie(poin, n, pivot)
int *poin;
int n;
int *pivot;
{
	int i=0, j= n-1, k, t1;
	int *next;
	
	k= 0;
	while(k<vergsize) {
		temppivot[k]= pivot[k];
		k++;
	}
	
	next= poin+ vergsize*(n-1);
	
	while(i<=j) {
		while( vergfunc(poin, temppivot)== -1 ) {
			i++;
			poin+= vergsize;
		}
		while( vergfunc(next, temppivot)!= -1 ) {
			j--;
			next-= vergsize;
		}
		if(i<j) {
			/* wissel */
			if(vergsize==1) {
				t1= *poin;
				*poin= *next;
				*next= t1;
				poin++;
				next--;
			}
			else if(vergsize==2) {
				t1= poin[0];
				poin[0]= next[0];
				next[0]= t1;
				t1= poin[1];
				poin[1]= next[1];
				next[1]= t1;
				poin+= 2;
				next-= 2;
			}
			else {
				k= 0;
				while(k<vergsize) {
					t1= poin[k];
					poin[k]= next[k];
					next[k]= t1;
					k++;
				}
				poin+=vergsize;
				next-=vergsize;
			}
			i++;
			j--;
		}
	}
	return i;
}

void qsortNN( poin, n)
int *poin;
int n;
{
	int i, k, ok=0;
	int *pivot, *next;
	
	/* zoek pivot */
	if(n<3) {
		if(n==2) {
			next= poin+vergsize;
			k= vergfunc(poin, next);
			if(k==1) {
				k= 0;
				while(k<vergsize) {
					i= poin[k];
					poin[k]= next[k];
					next[k]= i;
					k++;
				}
			}
		}
		return;
	}
	else {
		next= poin+vergsize;
		for(i=1; i<n; ++i, next+=vergsize) {
			k= vergfunc(poin, next);
			if(k== 1) {
				pivot= poin;
				ok= 1;
				break;
			}
			else if(k== -1) {
				pivot= next;
				ok= 1;
				break;
			}
		}

		if( ok ) {
			k= partitie(poin, n, pivot);
			qsortNN(poin, k);
			qsortNN(poin+vergsize*k, n-k);
		}
	}
}

void qsortN( poin, n, size, func)
void *poin;
int n, size;
int (*func)();
{

	vergfunc= func;
	vergsize= size/4;

	if(4*vergsize!=size) {
		printf("wrong size in qsortN\n");
		return;
	}

	qsortNN(poin, n);
}


/* ********** PREFAB QSORT ********* */


#define VERGLONG2(e1, e2)	( (e1[0] > e2[0])? 1 : ( (e1[0] < e2[0]) ? -1 : ( (e1[1] > e2[1]) ? 1 : ( (e1[1] < e2[1]) ? -1 : 0 ) ) ) )


int partitie2(poin, n, pivot)
int *poin;
int n;
int *pivot;
{
	int i=0, j= n-1, k, t1;
	int *next;
	
	temppivot[0]= pivot[0];
	temppivot[1]= pivot[1];
	
	next= poin+ 2*(n-1);
	
	while(i<=j) {
		while( VERGLONG2(poin, temppivot)== -1 ) {
			i++;
			poin+= 2;
		}
		while( VERGLONG2(next, temppivot)!= -1 ) {
			j--;
			next-= 2;
		}
		if(i<j) {
			t1= poin[0];
			poin[0]= next[0];
			next[0]= t1;
			t1= poin[1];
			poin[1]= next[1];
			next[1]= t1;
			poin+= 2;
			next-= 2;
			i++;
			j--;
		}
	}
	return i;
}

void qsortN2( poin, n)
int *poin;
int n;
{
	int i, k, ok=0;
	int *pivot, *next;
	
	/* zoek pivot */
	if(n<3) {
		if(n==2) {
			next= poin+2;
			k= VERGLONG2(poin, next);
			if(k==1) {
				i= poin[0];
				poin[0]= next[0];
				next[0]= i;
				i= poin[1];
				poin[1]= next[1];
				next[1]= i;
			}
		}
	}
	else {
		next= poin+2;
		for(i=1; i<n; ++i, next+=2) {
			k= VERGLONG2(poin, next);
			if(k== 1) {
				pivot= poin;
				ok= 1;
				break;
			}
			else if(k== -1) {
				pivot= next;
				ok= 1;
				break;
			}
		}

		if( ok ) {
			k= partitie2(poin, n, pivot);
			qsortN2(poin, k);
			qsortN2(poin+2*k, n-k);
		}
	}
	sginap(1);
}





int verglong(e1, e2)
int *e1, *e2;
{
	if( e1[0] > e2[0] ) return 1;
	else if( e1[0] < e2[0] ) return -1;
	else if( e1[1] > e2[1] ) return 1;
	else if( e1[1] < e2[1] ) return -1;

	return 0;
}

void main()
{
	long a[10][2];
	

	a[0][0]= 5;
	a[1][0]= 5;
	a[2][0]= 9;
	a[3][0]= 5;
	a[4][0]= 1;
	a[5][0]= 5;
	a[6][0]= 3;
	a[7][0]= 2;
	a[8][0]= 6;
	a[9][0]= 1;

	a[0][1]= 1;
	a[1][1]= 2;
	a[2][1]= 3;	
	a[3][1]= 4;
	a[4][1]= 5;
	a[5][1]= 1;
	a[6][1]= 2;
	a[7][1]= 3;
	a[8][1]= 4;
	a[9][1]= 5;
	
	/* qsortN(a, 10, 8, verglong); */
	qsortN2(a, 10);
	
	printf("%d %d %d %d %d %d %d %d %d %d\n", a[0][0], a[1][0], a[2][0], a[3][0], a[4][0], a[5][0], a[6][0], a[7][0], a[8][0], a[9][0]);
	printf("%d %d %d %d %d %d %d %d %d %d\n", a[0][1], a[1][1], a[2][1], a[3][1], a[4][1], a[5][1], a[6][1], a[7][1], a[8][1], a[9][1]);
	
	exit(0);
}

