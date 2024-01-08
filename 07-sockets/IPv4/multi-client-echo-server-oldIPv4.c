// Operating Systems: sample code  (c) Tomáš Hudec
// Sockets
// socket(2), bind(2), listen(2), accept(2)
//
// multi-client echo server

// Modified: 2011-01-11, 2013-12-19. 2016-11-27

#include <stdio.h>		// basic I/O routines
#include <sys/types.h>		// standard system types
#include <netinet/in.h>		// internet address structures
#include <sys/socket.h>		// socket interface functions
#include <netdb.h>		// host to IP resolution
#include <stdlib.h>		// exit
#include <string.h>		// memset, memcpy
#include <unistd.h>		// read, write, table size calculations
#include <sys/time.h>		// timeout values

#define	PORT		5061	// port of the echo server
#define	BUFLEN		1024	// buffer length

int main()
{
	int d;			// index counter for sdescriptor loop operations
	int rc;			// system calls return value
	int s;			// socket descriptor
	int cs;			// new connection's socket descriptor
	struct sockaddr_in sa;	// internet address struct
	struct sockaddr_in csa;	// client's address struct
	socklen_t size_csa;	// size of client's address struct
	char buf[BUFLEN + 1];	// buffer for incoming data
	ssize_t r;		// number of bytes read
	ssize_t w;		// number of bytes written
	fd_set rfd;		// set of open sockets
	fd_set ready_rfd;	// set of sockets waiting to be read
	int dtsize;		// size of file descriptors table

	// initiate machine's internet address structure
	// first clear out the struct, to avoid garbage
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;	// using internet address family
	sa.sin_port = htons(PORT);	// copy port number in network byte order
	// accept cnnections coming through any IP address that belongs to our host
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	// other possibilities: INADDR_LOOPBACK, inet_addr("10.11.212.13")

	// allocate a socket: internet address family, stream socket, default protocol (TCP)
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// bind the socket to the formed address
	rc = bind(s, (struct sockaddr *) &sa, sizeof(sa));
	if (rc) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// ask the OS to listen for incoming connections
	// specify that up to 5 pending connection requests will be queued by the OS
	// if we are not directly awaiting them using the accept(2), when they arrive
	rc = listen(s, 5);
	if (rc) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// calculate the size of the file descriptors table, the maximum opened descriptors
	dtsize = getdtablesize();
	if (dtsize > FD_SETSIZE)
		dtsize = FD_SETSIZE;

	// close all file descriptors, except stdin/stdout/stderr and our communication socket
	// this is done to avoid blocking on tty operations and such
	for (d = 3; d < dtsize; ++d)
		if (d != s)
			close(d);

	FD_ZERO(&rfd);		// clear the set
	FD_SET(s, &rfd);	// set our listening descriptor to be watched

	// enter an select-accept-read-write-close infinite loop
	while (1) {
		// the select(2) system call waits until any of the file descriptors specified
		// in the read, write and exception sets given to it is ready
		// to give data, send data, or is in an exceptional state, in respect
		// the call will wait for a given time before returning unless value is NULL is used
		// dtsize is the highest-numbered descriptor in any of the three sets, plus one
		memcpy(&ready_rfd, &rfd, sizeof(rfd));	// copy the set
		// ready_rfd will be modified by select(2) to include only ready descriptors 
		rc = select(dtsize, &ready_rfd, NULL, NULL, (struct timeval *) NULL);
		if (rc < 0) {
			perror("select");
			return EXIT_FAILURE;
		}

		// now, test the set for sockets ready for reading
		for (d = 3; d < dtsize; ++d) {
			if (!FD_ISSET(d, &ready_rfd))	// skip descriptors not in the set
				continue;
			if (d == s) {	// a new connection request arrived, accept it
				size_csa = sizeof(csa);		// maximum size, accept(2) will set the actual size
				cs = accept(s, (struct sockaddr *) &csa, &size_csa);
				if (cs < 0)			// check for errors, if any, ignore this connection
					continue;
				FD_SET(cs, &rfd);		// add the new socket to the set of watched sockets
			}
			else {		// new data from a client are ready
				r = read(d, buf, BUFLEN);
				if (0 == r) {		// if client closed the connection
					close(d);		// close the socket
					FD_CLR(d, &rfd);	// remove the socket from the set of watched sockets
				}
				else {			// if there are data to read
					w = write(d, buf, r);	// echo it back to the client
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
