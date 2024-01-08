// Operating Systems: sample code  (c) Tomáš Hudec
// Sockets
// socket(2), bind(2), listen(2), accept(2)
//
// echo server

#include <stdio.h>		// basic I/O routines
#include <sys/types.h>		// standard system types
#include <netinet/in.h>		// internet address structures
#include <sys/socket.h>		// socket interface functions
#include <netdb.h>		// host to IP resolution
#include <stdlib.h>		// exit
#include <string.h>		// memset, memcpy
#include <unistd.h>		// read, write

#define	PORT		5566	// port of the echo server
#define	BUFLEN		1024	// buffer length

int main()
{
	int rc;			// system calls return value
	int s;			// socket descriptor
	int cs;			// new connection's socket descriptor
	struct sockaddr_in sa;	// internet address struct
	struct sockaddr_in csa;	// client's address struct
	socklen_t size_csa;	// size of client's address struct
	char buf[BUFLEN + 1];	// buffer for incoming data
	ssize_t r;		// number of bytes read

	// print id

	// FIXME: port as argument

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

	// enter an accept-read-write-close infinite loop
	while (1) {
		// the accept(2) system call will wait for a connection
		// when one is established, a new socket will be created to form it, and
		// the csa variable will hold the address of the client that connected to us
		// the s socket, will still be available for future accept(2) statements
		size_csa = sizeof(csa);	// maximum size, accept(2) will set the actual size
		cs = accept(s, (struct sockaddr *) &csa, &size_csa);
		if (cs < 0)		// check for errors, if any, enter accept mode again
			continue;

		// we've got a new connection, do the job
		// FIXME: while the client does not close the connection:
		r = read(cs, buf, BUFLEN);
		// FIXME check for errors and for a close request
		// send back what we received
		write(cs, buf, r);
		// FIXME: handle errors

		// now close the connection
		close(cs);
	}
	return 0;
}
