// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// DNS: domain to IP translation
// IPv4/IPv6
// getaddrinfo(3), gai_strerror(3), freeaddrinfo(3), inet_ntop(3)

#include <stdio.h>			// basic I/O routines
#include <sys/types.h>			// standard system types
#include <netinet/in.h>			// internet address structures
#include <sys/socket.h>			// socket interface functions
#include <netdb.h>			// host to IP resolution
#include <arpa/inet.h>			// convert address to a string: inet_ntop
#include <stdlib.h>			// exit
#include <string.h>			// memset

int main(int argc, char *argv[])
{
	int rc;				// system calls return value
	struct addrinfo hints;		// host-to-IP and service translation
	struct addrinfo *servinfo, *si;	// host-to-IP and service translation
	char ipstr[INET6_ADDRSTRLEN];	// address as a string
	void *addr;			// pointer to the address for inet_ntop
	char ipver;			// the IP version '4' or '6'

	if (argc < 2) {
		fprintf(stderr, "Usage:\n"
				"  show-ip hostname\n");
		return EXIT_FAILURE;
	}

	memset(&hints, 0, sizeof(hints));	// clear out the struct
	hints.ai_family = AF_UNSPEC;		// use both IPv4/IPv6 (AF_INET for IPv4, AF_INET6 for IPv6)
	hints.ai_socktype = SOCK_STREAM;	// TCP stream socket (if not specified, multiple results will be returned)

	rc = getaddrinfo(argv[1], NULL, &hints, &servinfo);	// get the IP
	if (rc) {
		// print the error message using gai_strerror(3)
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
		return EXIT_FAILURE;
	}

	printf("IP addresses for %s:\n", argv[1]);

	// for all available addresses
	for (si = servinfo; NULL != si; si = si->ai_next) {
		// use different members for IPv4 and IPv6
		switch (si->ai_family) {
		case AF_INET:	// IPv4
			ipver = '4';
			addr = &((struct sockaddr_in *)si->ai_addr)->sin_addr;
			break;
		case AF_INET6:	// IPv6
			ipver = '6';
			addr = &((struct sockaddr_in6 *)si->ai_addr)->sin6_addr;
			break;
		default:
			fprintf(stderr, "  Unknown address family: %d.\n", si->ai_family);
			continue;
		}

		// convert the IP to a string and print it
		inet_ntop(si->ai_family, addr, ipstr, sizeof(ipstr));
		printf("  IPv%c: %s\n", ipver, ipstr);
	}

	freeaddrinfo(servinfo);	// free the linked list
	servinfo = NULL;

	return EXIT_SUCCESS;
}
