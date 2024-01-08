// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// Client: daytime
// IPv4/IPv6
// getaddrinfo(3), gai_strerror(3), freeaddrinfo(3)
// socket(2), bind(2), connect(2), read(2)/recv(2), close(2)

// Modified: 2016-11-25, 2023-04-14

#include <stdio.h>			// basic I/O routines
#include <sys/types.h>			// standard system types
#include <sys/socket.h>			// socket interface functions
#include <netdb.h>			// host to IP resolution
#include <stdlib.h>			// exit
#include <string.h>			// memset, memcpy
#include <unistd.h>			// read, write
#include <errno.h>

#define	BUFLEN		1024		// maximum response size
#define	HOSTNAMELEN	40		// maximum host name length
#define	SERVICE		"daytime"	// or "13" (port of daytime server)

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
	char buf[BUFLEN + 1];		// buffer server answer
	char *pc;			// pointer into the buffer
	struct addrinfo hints;		// host-to-IP and service translation

	// check there are enough parameters
	if (argc < 2) {
		fprintf(stderr, "Missing host name.\n");
		return EXIT_FAILURE;
	}

	// address resolution ("domain.name" or "IP" to IP)
	memset(&hints, 0, sizeof(hints));	// clear out the struct
	hints.ai_family = AF_UNSPEC;		// both IPv4/IPv6 (AF_INET for IPv4, AF_INET6 for IPv6)
	hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
	rc = getaddrinfo(argv[1], SERVICE, &hints, &servinfo);	// get the IP and port numbers
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
		// as we use atexit(3), there's no need to call freeaddrinfo(3) here
		return EXIT_FAILURE;
	}

	// now connect to the remote server
	rc = connect(s, servinfo->ai_addr, servinfo->ai_addrlen);
	if (rc) {
		perror("connect");
		return EXIT_FAILURE;
	}

	// the servinfo is no more needed
	cleanup_servinfo();

	// start reading the socket while read(2) is returning something
	// rc == 0: the server closed the connection or a buffer is full
	pc = buf;
	while ((rc = read(s, pc, BUFLEN - (pc - buf))) > 0)
		pc += rc;
	// alternative loop using recv(2)
	//while ((rc = recv(s, pc, BUFLEN - (pc - buf), 0)) > 0)
	//	pc += rc;

	close(s);	// close the socket

	if (rc < 0) {	// read error
		perror("read");
		return EXIT_FAILURE;
	}

	// pad a null character to the end of the result
	*pc = '\0';

	// print the result
	printf("Time: %s", buf);

	return EXIT_SUCCESS;
}
