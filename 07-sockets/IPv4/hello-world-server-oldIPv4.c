// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// Single-Client Server: hello-world
// struct sockaddr_in, socket(2), bind(2), listen(2), accept(2), read(2)/write(2), close(2)

// Modified: 2016-11-27

#include <stdio.h>		// basic I/O routines
#include <sys/types.h>		// standard system types
#include <netinet/in.h>		// internet address structures
#include <sys/socket.h>		// socket interface functions
#include <netdb.h>		// host to IP resolution
#include <stdlib.h>		// exit
#include <string.h>		// memset, memcpy
#include <unistd.h>		// read, write

#define	PORT		5050	// port of "hello world" server
// Note: ports below 1024 are RESERVED (for super-user)
#define	LINE	"Hello world\n"	// what to say to our clients

int main(int argc, char *argv[])
{
	int rc;			// system calls return value
	int s;			// socket descriptor
	int cs;			// new connection's socket descriptor
	struct sockaddr_in sa;	// server's internet address struct
	struct sockaddr_in csa;	// client's internet address struct
	socklen_t size_csa;	// size of client's address struct
	ssize_t w;		// return value of write(2)

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

	// enter an accept-write-close infinite loop
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
		w = write(cs, LINE, sizeof(LINE));

		close(cs);		// now close the connection
	}
	return EXIT_SUCCESS;
}
