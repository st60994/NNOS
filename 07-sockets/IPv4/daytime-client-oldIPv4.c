// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// Client: daytime
// IPv4
// struct sockaddr_in, gethostbyname(3),
// socket(2), bind(2), listen(2), accept(2), read(2)/write(2), close(2)

// Modified: 2016-11-25

#include <stdio.h>		// basic I/O routines
#include <sys/types.h>		// standard system types
#include <sys/socket.h>		// socket interface functions
#include <netdb.h>		// host to IP resolution
#include <stdlib.h>		// exit
#include <string.h>		// memset, memcpy
#include <unistd.h>		// read, write
#include <errno.h>

#define	HOSTNAMELEN	40	// maximum host name length
#define	BUFLEN		1024	// maximum response size
#define	PORT		13	// port of daytime server

int main(int argc, char *argv[])
{
	int rc;			// system calls return value
	int s;			// socket descriptor
	char buf[BUFLEN + 1];	// buffer server answer
	char *pc;		// pointer into the buffer
	struct sockaddr_in sa;	// internet address struct
	struct hostent *hen;	// host-to-IP translation

	// check there are enough parameters
	if (argc < 2) {
		fprintf(stderr, "Missing host name.\n");
		return EXIT_FAILURE;
	}
	// address resolution ("domain.name" or "IP" to IP)
	hen = gethostbyname(argv[1]);
	if (!hen) {
		perror("Couldn't resolve the host name.");
		return EXIT_FAILURE;
	}
	// initiate machine's internet address structure
	// first clear out the struct, to avoid garbage
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;	// internet address family
	sa.sin_port = htons(PORT);	// copy port number in network byte order
	// copy (the first resolved) IP address into the address struct
	memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);

	// allocate a socket: internet address family, stream socket, default protocol (TCP)
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// now connect to the remote server
	rc = connect(s, (struct sockaddr *) &sa, sizeof(sa));
	if (rc) {
		perror("connect");
		return EXIT_FAILURE;
	}

	// start reading the socket while read(2) is returning something
	// rc == 0: the server closed the connection or a buffer is full
	pc = buf;
	while ((rc = read(s, pc, BUFLEN - (pc - buf))) > 0) {
		pc += rc;
	}

	close(s);	// close the socket

	if (rc < 0) {	// read error
		perror("read");
		return EXIT_FAILURE;
	}

	// pad a null character to the end of the result
	*pc = '\0';

	// print the result
	printf("Time: %s\n", buf);

	return EXIT_SUCCESS;
}
