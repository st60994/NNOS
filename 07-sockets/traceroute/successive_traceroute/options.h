#include <netdb.h>		// sa_family_t, in_port_t
#include <unistd.h>		// getopt

#define ME			"traceroute"
#define VERSION			"1.0"

#define DEF_AF			AF_INET
#define FIRST_TTL		1
#define DEF_TTL			30
#define DEF_PROBES		3
#define DEF_TIMEOUT		5.0
#define MAX_TIMEOUT		300.0
#define DEF_RESOLVE		1
#define DEF_PACKET_SIZE		40
#define MAX_PACKET_LEN		65000

#define DEF_DEST_PORT		53
//#define DEF_DEST_PORT		5121
#define	DEF_DEST_PORT_BASE	33434
#define MAX_DEST_PORT		65000

#define MAX_TTL			255
#define MAX_PROBES		10

// text
#define __TEXT(X)	#X
#define _TEXT(X)	__TEXT(X)

// debug
// #define DEBUG(...)	if (opts.debug) fprintf(stderr, __VA_ARGS__)
#define DEBUG_COLOR_D	"\033[36m"
#define DEBUG_COLOR_N	"\033[0m"
#define DEBUG(...)	if (opts.debug) { fprintf(stderr, DEBUG_COLOR_D); fprintf(stderr, __VA_ARGS__); fprintf(stderr, DEBUG_COLOR_N); }
#define DEBUGNL(...)	if (opts.debug) { fprintf(stderr, DEBUG_COLOR_D); fprintf(stderr, __VA_ARGS__); fprintf(stderr, DEBUG_COLOR_N); fprintf(stderr, "\n"); }


struct options {
	sa_family_t af;		// default address family
	int probes;		// max. probes per TTL
	int first_ttl;		// first ttl
	int max_ttl;		// max. ttl
	int noresolve;		// resolve?
	int dest_port_seq;	// use different destination port?
	in_port_t dest_port;	// destination port
	double timeout;		// timeout
	size_t packet_len;	// packet length
	int debug;		// debug messages?
	int verbose;		// verbosity
};

extern struct options opts;

void parse_options(int argc, char *argv[]);

