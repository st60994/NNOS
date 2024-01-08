// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// Single-Client Server: hello-world
// IPv4/IPv6
// getaddrinfo(3), gai_strerror(3), freeaddrinfo(3)
// socket(2), setsockopt(2), bind(2), listen(2), accept(2), write(2)/send(2), close(2)

// Modified: 2016-11-28

#include <stdio.h>			// basic I/O routines
#include <sys/types.h>			// standard system types
#include <netinet/in.h>			// internet address structures
#include <sys/socket.h>			// socket interface functions
#include <netdb.h>			// host to IP resolution
#include <stdlib.h>			// exit
#include <string.h>			// memset, memcpy
#include <unistd.h>			// read, write

#define	SERVICE		"5050"		// port of our "hello world" server
// Note: ports below 1024 are RESERVED (for super-user)

#define	QUEUE_SIZE	5		// how many pending connections to keep in the queue

#define	LINE	"Hello world!\n"	// what to say to our clients

struct addrinfo *servinfo = NULL;	// host-to-IP translation

// this should be called upon exit
void cleanup_servinfo(void)
{
	if (NULL != servinfo) {
		freeaddrinfo(servinfo);
		servinfo = NULL;
	}
}

int main(int argc, char *argv[])
{
	int rc;				// system calls return value
	int s;				// socket descriptor
	int cs;				// new connection's socket descriptor
	struct sockaddr_storage client;	// client's internet address info
	socklen_t client_addr_size;	// size of client's address struct
	struct addrinfo hints;		// host-to-IP and service translation
	int yes = 1;			// for socket releasing
	ssize_t w;			// the number of bytes written by write(2)/send(2)

	// set the listening address
	memset(&hints, 0, sizeof(hints));	// clear out the struct
	hints.ai_family = AF_UNSPEC;		// use both IPv4/IPv6 (AF_INET for IPv4, AF_INET6 for IPv6)
	hints.ai_socktype = SOCK_STREAM;	// TCP stream socket
	hints.ai_flags = AI_PASSIVE;		// to fill in the local IP (if the first arg is NULL)
	rc = getaddrinfo(NULL, SERVICE, &hints, &servinfo);	// set the IP and port numbers
	if (rc) {
		// print the error message using gai_strerror(3)
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
		return EXIT_FAILURE;
	}
	// servinfo should be freed upon exit using freeaddrinfo(3)
	atexit(cleanup_servinfo);

	// allocate a socket, use address family, socket type and protocol from servinfo
	s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (s < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// reuse the socket in case of the "Address already in use" error message
	rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (rc) {
		perror("setsockopt");
		return EXIT_FAILURE;
	} 

	// bind the socket to the address and port
	rc = bind(s, servinfo->ai_addr, servinfo->ai_addrlen);
	if (rc) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// the servinfo is no more needed
	cleanup_servinfo();

	// ask the OS to listen for incoming connections
	// specify that up to QUEUE_SIZE pending connection requests will be queued by the OS
	// if we are not directly awaiting them using the accept(2), when they arrive
	rc = listen(s, QUEUE_SIZE);
	if (rc) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// enter an accept-write-close infinite loop
	while (1) {
		// the accept(2) system call will wait for a connection
		// when one is established, a new socket will be created to form it, and
		// the client variable will hold the address of the client that connected to us
		// the s socket, will still be available for future accept(2) statements
		client_addr_size = sizeof(client);	// maximum size, accept(2) will set the actual size
		cs = accept(s, (struct sockaddr *) &client, &client_addr_size);
		if (cs < 0)			// check for errors, if any, enter accept mode again
			continue;

		// we've got a new connection, do the job
		w = write(cs, LINE, sizeof(LINE));
		// alternative using send(2)
		// w = send(cs, LINE, sizeof(LINE), 0);
		if (w < 0)
			perror("write/send");

		close(cs);			// now close the connection
	}
	return EXIT_SUCCESS;
}
