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

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <local/network.h>

#define malloc(x) mallocN(x,"malloc")
#define free(x) freeN(x)
#define calloc(x,y) callocN((x)*(y),"calloc")

/*

zipfork "cc tracesd.c -o tracesd -lmalloc >/dev/console"                                          
zipfork "ed /data/cke/pics/0201 /data/test/pira.0001 >/dev/console"                                          

*/

long accept_connection(long sock, struct in_addr * address)
{

	int length;
	long connection;
	struct sockaddr_in sin;

	length = sizeof(sin);
	connection = accept(sock, &sin, &length);
	if (connection < 0) {
		perror("accept");
		return(-1);
	}

	/*printf("Connection from host %s, port %u\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));*/
	if (address) *address = sin.sin_addr;
	
	fflush(stdout);

	return(connection);
}


long init_recv_socket(long port)
{
	long sock;
	struct sockaddr_in sin;

	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) {
		perror("Error opening socket");
		return(-1);
	}

	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	/* bind socket data structure to this socket */

	if (bind (sock,&sin,sizeof(sin))) {
		perror("Error binding socket");
		close(sock);
		return(-1);
	}

	if (listen(sock,5) == -1){
		perror("listen");
		close(sock);
		return(-1);
	}

	return(sock);
}


long connect_to_host(char * hostname, long port)
{
	long sock;
	struct hostent *hp;
	struct sockaddr_in sin;

	sock = socket (AF_INET,SOCK_STREAM,0);
	if (sock < 0) {
		perror("Error opening socket");
		return(-1);
	}

	/* initialize socket address structure */
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	hp = gethostbyname(hostname);
	if (hp == NULL) {
		herror(hostname);
		close(sock);
		return(-1);
	}
	bcopy (hp->h_addr, &(sin.sin_addr.s_addr), hp->h_length);

	/* connect to remote host */

	if (connect(sock,&sin,sizeof(sin)) < 0) {
		close(sock);
		perror("Can't connect to server");
		return(-1);
	}
	/*printf("Connection established\n");*/
	fflush(stdout);
	return(sock);
}


long net_write(long connection, char * buf, long size){
	int * temp, * _temp;
	long ret = 0, written, to_write, retries = 3;
	
	_temp = temp = (int *) malloc(size + sizeof(int));
	if (temp == 0) return (-1);
	
	temp[0] = size;
	memcpy(temp + 1, buf, size);

	to_write = size + sizeof(int);
	written = write(connection, temp, to_write);

	while (written != to_write) {
		if (written == -1) written = 0;
		if (retries-- == 0) break;
		to_write -= written;
		temp = (int *) (((char *) temp) + written);
		sginap (3);
		written = write(connection, temp, to_write);
	}
	
	if (written != to_write) {
		printf ("requested %d written %d\n", to_write, written);
		printf("error %d\n", errno);
		perror("net_write");
		ret = -1;
	}

/*	if (write(connection, temp, size + sizeof(int)) != size + sizeof(int)){
		printf("error %d\n", errno);
		perror("net_write");
		ret = -1;
	}*/
	
	free(_temp);
	return (ret);
}


long net_read_old(long connection, void * buf, long size)
{
	long count, i;
	int len;

	errno = 0;
	count = read(connection, &len, sizeof(int));
	if (count == -1){
		if (errno != EWOULDBLOCK) perror("net_read_length");
	} else if (count == sizeof(int)) {
		if (size < len) {
			printf("Buffer to small: %d > %d\n", len, size);
			count = 0;
		} else {
			count = read(connection, buf, len);
			if (count == -1) perror("net_read_data");
			else if (count == 0) {
			} else if (count != len) {
				perror(" perror says:");
				/*if (errno == EWOULDBLOCK) {*/
					for (i = 0 ; i < 10; i++) {
						Sginap(20);
						printf("RETRYING\n\n");
					}
					count += read(connection, ((char * ) buf) + count, len - count);
				/*}*/
				if (count != len) {
					*(((char *) buf) + count) = 0;
					printf("\n net_read count (%d) <> len (%d)\n [%s]\n Danger ahead\n", count, len, buf);
					perror(" perror says:");
					printf("\n");
					count = -1;
				}
			}
		}
	} else if (count != 0) {
		printf("net_read count = %d\n Danger ahead\n", count);
		count = -1;
	}
	
	return (count);
}

long net_read(long connection, void * buf, long size)
{
	long count, i;
	int len, new;
		
	/* deze routine moet anders: Bij de tweede keer lezen (zodra de long
	 * binnen is), moet de filepointer op blocking gezet worden. Om te
	 * voorkomen dat alles vastloopt, moet er een interrupt gezet
	 * worden (van +- 1 sec).
	 */

	errno = 0;

	count = read(connection, &len, sizeof(int));
	if (count == -1){
		if (errno != EWOULDBLOCK) perror("net_read_length");
	} else if (count == sizeof(int)) {
		if (size < len) {
			fprintf(stderr, "Buffer to small: %d > %d\n", len, size);
			count = 0;
		} else {
			count = 0;
			for (i = 0; i < 10; i++) {
				new = read(connection, ((char *) buf) + count, len - count);
				if (new <= 0) {
					count = new;
					break;
				}
				
				count += new;
				if (count == len) break;
				Sginap(10);
				fprintf(stderr, "Read: %d. Need: %d. Retrying\n", count, len);
			}
			
			if (count != len) {
				printf("read returned %d ", count);
				if (count == -1) {
					perror("net_read_data");
				} else {
					perror("net_read_error");
					count = 0;
				}
			}
		}
	} else if (count != 0) {
		printf("net_read count = %d\n Danger ahead\n", count);
		count = -1;
	}
	
	return (count);
}

