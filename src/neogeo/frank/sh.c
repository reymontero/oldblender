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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

/* 
bindkeyo -r f1,'cc sh.c -o shared \n'
bindkeyo -r f2,'shared \n'
*/

main_shared()
{
	long shared , i;
	long *mapped;

	printf("%d\n",sizeof(int));
	/* reserveer een structuur voor het gedeelde geheugengebied van lengte xxxx*/
	printf("%d\n",shared = shmget(0,100000,SHM_R | SHM_W|SHM_DEST));

	/* map die in je geheugenbereik */
	/* printf("%d\n",mapped = shmat(shared,0,SHM_RDONLY)); */
	printf("%d\n",mapped = shmat(shared,0,0));

	for (i = 0 ; i <1024 ; i++) mapped[i] = 0;
	/* haal 'm weer eruit */
	printf("%d\n",shmdt(mapped));

	/* en delete de structuur */
	printf("%d\n",shmctl(shared,IPC_RMID));

}

main()
{
	long i,j,size,rt;
	long *mem, *_mem;

	size = 5000000;
	_mem = (long *) malloc(size);
	if (_mem == 0) exit(0);
	
	for (j = 3 ; j> 0 ; j --){
		mem = _mem;
		for (i = size / 4 ; i > 0 ; i--){
			*mem++ = i;
		}
	}
	msync(_mem,size,MS_ASYNC | MS_INVALIDATE);
	printf("waiting\n");
	sginap(500);
	for (j = 3 ; j> 0 ; j --){
		mem = _mem;
		for (i =  size / 4 ; i> 0 ; i--){
			rt = *mem++;
		}
	}
	msync(_mem,size,MS_ASYNC | MS_INVALIDATE);
	sginap(500);
}

