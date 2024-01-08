#include <sys/types.h>		/* standard system types        */
#include <sys/socket.h>		/* socket interface functions   */
#include <netdb.h>		/* host to IP resolution        */

#define MAX_TTL			255
#define MAX_PROBES		10

#ifndef _GNU_SOURCE
#	ifndef NI_IDN
#		define NI_IDN 0
#	endif
#endif

// text
#define __TEXT(X)	#X
#define _TEXT(X)	__TEXT(X)

/*
  ip(7)
	struct sockaddr_in {
		sa_family_t	sin_family;	// address family: AF_INET
		uint16_t	sin_port;	// port in network byte order
		struct in_addr	sin_addr;	// internet address
	};
	struct in_addr {
		uint32_t	s_addr;		// address in network byte order
	};
  socket.h(7)
	struct sockaddr {
		sa_family_t	sa_family;	// Address family.
		char		sa_data[14];	// other data
	};
*/

// union of all types of socket address families
union common_sockaddr {
	struct sockaddr sa;		// common basic type
	struct sockaddr_in sin;		// AF_INET
	struct sockaddr_in6 sin6;	// AF_INET6
};
typedef union common_sockaddr sockaddr_any;


// one probe
struct probe_info {
	// per probe
	// int id;			// probe id
	in_port_t dest_port;		// target host (if dst port changes)
	sockaddr_any local_addr;	// local addres to bind
	sockaddr_any recv_addr;		// host answered ICMP TTL
	int recv_ttl;			// ttl got
	int socket;			// socket used
	double send_time;		// sending time
	double recv_time;		// receiving time
	char *data;			// packet data
	size_t data_len;		// data length
	char err_str[16];		// error string (!H, !N, ...)
};

// all probes
struct traceroute_info {
	sa_family_t af;			// address family AF_INET || AF_INET6
	// global
	// char *dest;
	sockaddr_any dest_addr;		// target host info
	// char *dest_hostname;		// target hostname string
	// char *dest_ipstr;		// target IP string
	int final_probe;		// host reached || error got (!H, !N, etc.)
	int probes_per_ttl;		// max. probes per TTL
	int max_ttl;			// max. ttl
	//in_port_t first_local_port;	// for binding w/specific local port
	struct probe_info probes[MAX_TTL*MAX_PROBES];	// per probe info
};

