#include <sys/types.h>		/* standard system types        */
#include <sys/socket.h>		/* socket interface functions   */
#include <netdb.h>		/* host to IP resolution        */

#ifndef _GNU_SOURCE
#	ifndef NI_IDN
#		define NI_IDN 0
#	endif
#endif

#define MAX_NAME_LEN	255	// see RFC

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

#define	PROBE_UNKNOWN		0	// unknown status
#define	PROBE_OK		1	// ok, targed reached or TTL expired
#define	PROBE_ERR_INIT		2	// fatal error (malloc failed)
#define	PROBE_ERR_SEND		3	// less fatal error: socket, bind, setsockopt, connect, send
#define	PROBE_ERR_RECV		4	// host or net unreachable
#define	PROBE_ERR_TIMEOUT	5	// timeout expired

// one probe
struct probe_info {
	// per probe
	int probe_result;		// result code
	in_port_t dest_port;		// target host (if dst port changes)
	sockaddr_any local_addr;	// local addres to bind
	sockaddr_any recv_addr;		// host answered ICMP TTL
	char *recv_hostname;		// hostname string
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
	sockaddr_any dest_addr;		// target host info
	int final_probe;		// host reached || error got (!H, !N, etc.)
	int probes_per_ttl;		// max. probes per TTL
	int max_ttl;			// max. ttl
	//in_port_t first_local_port;	// for binding w/specific local port
#if PROBES_DYNAMIC == 1
	struct probe_info *probes;	// per probe info
#else
	struct probe_info probes[MAX_TTL*MAX_PROBES];	// per probe info
#endif
};

