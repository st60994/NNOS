/*
 * simple traceroute
 *
 */

// #define _GNU_SOURCE		/* kvůli NI_IDN */

#include <stdio.h>		/* Basic I/O routines           */
#include <sys/types.h>		/* standard system types        */
#include <netinet/in.h>		/* Internet address structures  */
#include <sys/time.h>		/* for timeout values           */
#include <unistd.h>		/* for table size calculations  */
#include <stdlib.h>		/* exit */
#include <string.h>		/* memset, memmove, memcpy */

/* According to POSIX.1-2001 */
#include <sys/select.h>
#include <errno.h>

/* getnameinfo, getaddrinfo */
#include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>		// fcntl
#include <poll.h>		// poll

#include <netinet/ip_icmp.h>	// ICMP error message codes

#include <linux/types.h>	// required for linux/errqueue.h
#include <linux/errqueue.h>	// SO_EE_ORIGIN_ICMP

#include "traceroute.h"

#include <locale.h>		// locales
/*
 * #include <sys/file.h>
 * #include <sys/ioctl.h>
 */

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
};

struct options opts = {
	.af =			AF_INET,
	.probes =		DEF_PROBES,
	.first_ttl =		1,
	.max_ttl =		DEF_TTL,
	.noresolve =		0,
	.dest_port_seq =	1,
	.dest_port =		0,
	.timeout =		DEF_TIMEOUT,
	.packet_len =		DEF_PACKET_SIZE,
};

struct traceroute_info tr;

// time in floating point type double
double get_time(void)
{
	struct timeval tv;
	double d;

	gettimeofday(&tv, NULL);

	d = ((double) tv.tv_usec) / 1000000.0 + (unsigned long) tv.tv_sec;

	return d;
}

// nastaví adresu cíle, svůj port
int traceroute_init(char *target_host, sa_family_t af) {
	struct addrinfo hints, *res, *p;
	int rc;

	memset(&tr, 0, sizeof(tr));	// init the whole structure
	tr.final_probe = -1;

	memset(&tr.dest_addr, 0, sizeof(tr.dest_addr));	// init
	tr.af = af;
	tr.dest_addr.sa.sa_family = af;

	// convert given host/IP to network byte odered data

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = af;	// AF_UNSPEC, AF_INET or AF_INET6
	// hints.ai_socktype = SOCK_DGRAM;	// SOCK_STREAM;
	// node, service, hints, res
	if ((rc = getaddrinfo(target_host, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
		return 1;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) {	// IPv4
			// char ipstr[INET_ADDRSTRLEN];
			if (p->ai_addrlen > sizeof(tr.dest_addr)) {
				fprintf(stderr, "not enough space in dest_addr (struct sockaddr_in) while copying from (struct addrinfo).ai_addr\n");
				return 1;
			}
			memcpy(&tr.dest_addr, p->ai_addr, p->ai_addrlen);
			// inet_ntop(p->ai_family, p->ai_addr, ipstr, sizeof(ipstr));
			// tr.dest_ipstr = strdup(ipstr);
			tr.dest_addr.sa.sa_family = p->ai_family;
		}
		/*
		else if (p->ai_family == AF_INET6) {	// IPv6 stuff here
			char ipstr6[INET6_ADDRSTRLEN];
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			// convert the IP to a string and print it:
			inet_ntop(p->ai_family, &(ipv6->sin6_addr), ipstr6, sizeof(ipstr6));
			tr.dest_ipstr = strdup(ipstr6);
		}
		*/
	}

	tr.dest_addr.sin.sin_port = htons(opts.dest_port);

	freeaddrinfo(res);

	return 0;
}

int traceroute_init_probe(int pn)
{
	/*
	tr.probes = realloc(tr.probes, (pn+1)*sizeof(struct probe_info));
	if (tr.probes == NULL)
		return 1;
	*/
	memset(&tr.probes[pn], 0, sizeof(struct probe_info));
	return 0;
}

int bind_socket(int pn)
{
	int rc;

	memset(&tr.probes[pn].local_addr, 0, sizeof(tr.probes[pn].local_addr));
	tr.probes[pn].local_addr.sin.sin_family = tr.af;
	tr.probes[pn].local_addr.sin.sin_addr.s_addr = INADDR_ANY;
	// tr.probes[pn].local_addr.sin.sin_port = 0;
	/* bind the socket to the newly formed address */
	rc = bind(tr.probes[pn].socket, &tr.probes[pn].local_addr.sa, sizeof(tr.probes[pn].local_addr));
	if (rc) {
		perror("bind");
		return 4;
		// exit(4);
	}

	return 0;
}
// inicializuje sockety???
// pošle packet s nastaveným TTL
int traceroute_send(int pn, int ttl) {
	int val;
	sockaddr_any dest_addr;

	// allocate a socket
	tr.probes[pn].socket = socket(tr.af, SOCK_DGRAM, IPPROTO_UDP);
	if (tr.probes[pn].socket < 0) {
		perror("socket: allocation failed");
		// exit(3);
		return 3;
	}

	// bind a local port
	if (bind_socket(pn)) {
		return 4;
		// exit(4);
	}

	// Set TOS? Set Don't Fragment?

	// use_timestamp()?
	// int val = 1;
	// setsockopt(tr.probes[pn].socket, SOL_SOCKET, SO_TIMESTAMP, &val, sizeof(val));

	// use_recv_ttl:
	val = 1;
	setsockopt(tr.probes[pn].socket, SOL_IP, IP_RECVTTL, &val, sizeof(val));	// man 7 ip

	// non blocking socket?
	fcntl(tr.probes[pn].socket, F_SETFL, O_NONBLOCK);

	// set TTL
	if (setsockopt(tr.probes[pn].socket, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("setsockopt: IP_TTL");
		// exit(4);
		return 5;
	}

	// set dest_addr
	memcpy(&dest_addr, &tr.dest_addr, sizeof(dest_addr));
	if (opts.dest_port_seq) {	// set port
		tr.probes[pn].dest_port = opts.dest_port + pn;
		dest_addr.sin.sin_port = htons(tr.probes[pn].dest_port);
	}

	// connect for UDP only associates destination with the socket
	if (connect(tr.probes[pn].socket, &dest_addr.sa, sizeof(dest_addr)) < 0) {
		perror("connect");
		// exit(5);
		return 6;
	}

	// use_recv_err:
	val = 1;
	if (setsockopt(tr.probes[pn].socket, SOL_IP, IP_RECVERR, &val, sizeof(val)) < 0) {
		perror("setsockopt: IP_RECVERR");
		// exit(4);
		return 5;
	}

	// set data
	tr.probes[pn].data_len = opts.packet_len;
	tr.probes[pn].data = calloc(1, tr.probes[pn].data_len);
	for (val = 0; val < tr.probes[pn].data_len; val++)
		tr.probes[pn].data[val] = 0x40 | (val & 0x3F);

	// set sending time
	tr.probes[pn].send_time = get_time();

	// send packet
	if (send(tr.probes[pn].socket, tr.probes[pn].data, tr.probes[pn].data_len, 0) < 0) {
		perror("send");
		// exit(5);
		return 6;
	}

	// set poll now?

	return 0;
}

// parse ICMP response
void parse_icmp_response(int pn, int type, int code, int info)
{
	char *str = "";
	char buf[sizeof(tr.probes[pn].err_str)];

	if (type == ICMP_TIME_EXCEEDED) {
		if (code == ICMP_EXC_TTL)
			return;
	} else if (type == ICMP_DEST_UNREACH) {

		switch (code) {
			case ICMP_UNREACH_NET:
			case ICMP_UNREACH_NET_UNKNOWN:
			case ICMP_UNREACH_ISOLATED:
			case ICMP_UNREACH_TOSNET:
				str = "!N";
				break;

			case ICMP_UNREACH_HOST:
			case ICMP_UNREACH_HOST_UNKNOWN:
			case ICMP_UNREACH_TOSHOST:
				str = "!H";
				break;

			case ICMP_UNREACH_NET_PROHIB:
			case ICMP_UNREACH_HOST_PROHIB:
			case ICMP_UNREACH_FILTER_PROHIB:
				str = "!X";
				break;

			case ICMP_UNREACH_PORT:
				/*  dest host is reached   */
				str = NULL;
				break;

			case ICMP_UNREACH_PROTOCOL:
				str = "!P";
				break;

			case ICMP_UNREACH_NEEDFRAG:
				snprintf(buf, sizeof(buf), "!F-%d", info);
				str = buf;
				break;

			case ICMP_UNREACH_SRCFAIL:
				str = "!S";
				break;

			case ICMP_UNREACH_HOST_PRECEDENCE:
				str = "!V";
				break;

			case ICMP_UNREACH_PRECEDENCE_CUTOFF:
				str = "!C";
				break;

			default:
				snprintf(buf, sizeof(buf), "!<%u>", code);
				str = buf;
				break;
		}
	}

	if (str && !*str) {
		snprintf(buf, sizeof(buf), "!<%u-%u>", type, code);
		str = buf;
	}

	if (str) {
		strncpy(tr.probes[pn].err_str, str, sizeof(tr.probes[pn].err_str));
		tr.probes[pn].err_str[sizeof(tr.probes[pn].err_str) - 1] = '\0';
	}

	tr.final_probe = pn;

	return;
}


// parse the answer possibly with ICMP TTL error message
void parse_cmsg(int pn, struct msghdr *msg)
{
	struct cmsghdr *cm;

	for (cm = CMSG_FIRSTHDR(msg); cm; cm = CMSG_NXTHDR(msg, cm)) {

		if (cm->cmsg_level == SOL_SOCKET) {

			// get time
			if (cm->cmsg_type == SO_TIMESTAMP) {
				struct timeval *tv = (struct timeval *) CMSG_DATA(cm);
				tr.probes[pn].recv_time = tv->tv_sec + tv->tv_usec / 1000000.0;
			}

		}
		else if (cm->cmsg_level == SOL_IP) {

			if (cm->cmsg_type == IP_TTL)
				tr.probes[pn].recv_ttl = *((int *) CMSG_DATA(cm));

			else if (cm->cmsg_type == IP_RECVERR) {
				struct sock_extended_err *ee;

				ee = (struct sock_extended_err *) CMSG_DATA(cm);

				if (ee->ee_origin == SO_EE_ORIGIN_ICMP) {
					memcpy (&tr.probes[pn].recv_addr, SO_EE_OFFENDER(ee), sizeof(tr.probes[pn].recv_addr));
					parse_icmp_response(pn, ee->ee_type, ee->ee_code, ee->ee_info);
				}
			}

		}
		else if (cm->cmsg_level == SOL_IPV6) {
		
			// IPv6 stuff here

		}

	}

	if (!tr.probes[pn].recv_time)
		tr.probes[pn].recv_time = get_time();

	return;
}

int traceroute_receive(int pn, double timeout)
{
	struct pollfd fds[1];
	// int nfds = 1;
	int nfds;
	int poll_timeout;
	double now;
	struct msghdr msg;
	struct sockaddr_in response;		// host answered IP_RECVERR
	char control_buf[1500];
	struct iovec iov;
	char buf[1500];

	// poll = wait for a) timeout, or b) ICMP message, or c) answer
	fds[0].fd = tr.probes[pn].socket;
	fds[0].events = POLLIN | POLLERR;
	do {
       		now = get_time();
		poll_timeout = 1000 * ((tr.probes[pn].send_time + timeout) - now);
		if (poll_timeout < 0)
			return 1;	// timeout
		nfds = poll(fds, 1, poll_timeout);
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			perror("poll");
			return 3;	// error
		}
		if (nfds == 0)
			return 2;	// timeout
		if (!(fds[0].revents & (POLLIN | POLLERR)))
			return 4;	// error

		// receive the answer or ICMP message

		memset(&msg, 0, sizeof(msg));
		msg.msg_name = &response;			// host
		msg.msg_namelen = sizeof(response);
		msg.msg_control = control_buf;
		msg.msg_controllen = sizeof(control_buf);
		iov.iov_base = buf;
		iov.iov_len = sizeof(buf);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		// receive
		if (recvmsg(tr.probes[pn].socket, &msg, (fds[0].revents & POLLERR) ? MSG_ERRQUEUE : 0) < 0) {
			perror("recvmsg");
			continue;	// poll again? or poll only if errno == EAGAIN?
		}
		break;
	} while(1);

	// parse answer
	parse_cmsg(pn, &msg);	// err (if any), tstamp, ttl

	if (!(fds[0].revents & POLLERR)) {
		memcpy(&tr.probes[pn].recv_addr, &response, sizeof(tr.probes[pn].recv_addr));
		tr.final_probe = pn;
	}

	close(tr.probes[pn].socket);
	tr.probes[pn].socket = -1;

	// tr.probes[pn].done = 1;

	return 0;
}

static char addr2str_buf[INET6_ADDRSTRLEN];

static const char *addr2str(const struct sockaddr *addr)
{
	getnameinfo(addr, sizeof(*addr), addr2str_buf, sizeof(addr2str_buf), 0, 0, NI_NUMERICHOST);
	return addr2str_buf;
}

// get address of a host in string
static char nameip[1024+INET6_ADDRSTRLEN+4];
const char *traceroute_get_host(int pn)
{
	const char *str;

	if (!tr.probes[pn].recv_addr.sa.sa_family)
		str = "*";

	str = addr2str(&tr.probes[pn].recv_addr.sa);

	if (!opts.noresolve) {
		char name[1024];

		name[0] = '\0';
		getnameinfo(&tr.probes[pn].recv_addr.sa, sizeof(tr.probes[pn].recv_addr), name, sizeof(name), 0, 0, NI_IDN);
		//  foo on errors
		sprintf(nameip, "%s (%s)", name, str);
		str = nameip;
	}

	return str;
}

int do_traceroute(char *target_addr)
{
	int probe;		// probe counter
	int ttl;		// ttl counter
	int pn;
	int rc;
	int printed;

	if (traceroute_init(target_addr, opts.af) != 0) {
		// error reporting done inside
		return 1;
	};
	printf("traceroute to %s:%d%s (packet size %d bytes):\n",
		target_addr, opts.dest_port, opts.dest_port_seq ? "+" : "", opts.packet_len);
	for (ttl = opts.first_ttl; ttl <= opts.max_ttl; ttl++) {
		printf("%2u.", ttl); fflush(stdout);
		for (printed = 0, probe = 0; probe < opts.probes; probe++) {
			pn = (ttl-1)*opts.probes + probe;
			if (traceroute_init_probe(pn) != 0) {
				return 2;
			}
			if (traceroute_send(pn, ttl) != 0) {
				close(tr.probes[pn].socket);
				tr.probes[pn].socket = -1;
				return 3;
			}
			if ((rc = traceroute_receive(pn, opts.timeout)) == 0) {
				if (!printed) {
					printf(" %s", traceroute_get_host(pn));
					fflush(stdout);
					printed = 1;
				}
				if (tr.probes[pn].recv_time) {
					double diff = tr.probes[pn].recv_time - tr.probes[pn].send_time;
					printf("  %.3f ms", diff * 1000);
				}
				if (tr.probes[pn].err_str[0])
					printf(" %s", tr.probes[pn].err_str);
				fflush(stdout);
				// break;
			}
			else {
				close(tr.probes[pn].socket);
				tr.probes[pn].socket = -1;
				// printf(" *(%d)", rc);
				printf(" *");
				fflush(stdout);
			}
		}
		printf("\n");
		if (tr.final_probe >= 0) {
			break;
		}
	}
	return 0;
}

void usage(FILE *f)
{
	fprintf(f,
	"Use:\n"
	"  %s [-hV] [-f first_TTL] [-m max_TTL] [-n] [-w wait_timeout] "
	"[-q queries_per_TTL] [-U] [-p dst_port]"
	" host [packet_length]\n"
	"Options:\n"
	"  -f first_TTL  Start from the first_ttl hop (instead of " _TEXT(FIRST_TTL) ").\n"
	"  -m max_TTL    Set max. TTL to be reached. Default is " _TEXT(DEF_TTL) ". Max. is " _TEXT(MAX_TTL) ".\n"
	"  -n            Do not resolve IP addresses to domain names.\n"
	"  -w timeout    Set waiting time in seconds for a response. Default is %.1f s.\n"
	"  -q queries    Set the number of probes per each hop. Default is " _TEXT(DEF_PROBES) ". Max. is " _TEXT(MAX_PROBES) ".\n"
	"  -U            Use specific UDP destination port for tracerouting. See -p.\n"
	"  -p dst_port   Set UDP destination port (with -U, default is " _TEXT(DEF_DEST_PORT) ") or port base (without -U, default is " _TEXT(DEF_DEST_PORT_BASE) ").\n"
	"Arguments:\n"
	"  host          Hostname or IP address.\n"
	"  packet_length Lenght of a packet to send. Default is " _TEXT(DEF_PACKET_SIZE) ".\n"
	"",
	ME, DEF_TIMEOUT);
}

int main(int argc, char *argv[])
{
	int rc;			// return value storage
	int opt;		// options

	setlocale (LC_ALL, "");

	/* parse options */
	while ((opt = getopt(argc, argv, "hVf:m:nw:q:Up:")) != -1) {
		switch (opt) {
			case 'h':
				usage(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
				printf("%s version %s\n", ME, VERSION);
				exit(EXIT_SUCCESS);
			case 'f':
				opts.first_ttl = atoi(optarg);
				break;
			case 'm':
				opts.max_ttl = atoi(optarg);
				if (opts.max_ttl > MAX_TTL)
					opts.max_ttl = MAX_TTL;
				break;
			case 'n':
				opts.noresolve = 1;
				break;
			case 'w':
				opts.timeout = atof(optarg);
				if (opts.timeout > MAX_TIMEOUT)
					opts.timeout = MAX_TIMEOUT;
				if (opts.timeout < 0)
					opts.timeout = 0;
				break;
			case 'q':
				opts.probes = atoi(optarg);
				if (opts.probes > MAX_PROBES)
					opts.probes = MAX_PROBES;
				if (opts.probes < 1)
					opts.probes = 1;
				break;
			case 'U':
				opts.dest_port_seq = 0;
				if (!opts.dest_port)
					opts.dest_port = DEF_DEST_PORT;
				break;
			case 'p':
				opts.dest_port = atoi(optarg);
				if (opts.dest_port > MAX_DEST_PORT)
					opts.dest_port = 0;
				break;
			default:
				usage(stderr);
				exit(EXIT_FAILURE);
		}
	}

	// set correct destination port
	if (!opts.dest_port) {
		if (opts.dest_port_seq)
			opts.dest_port = DEF_DEST_PORT_BASE;
		else
			opts.dest_port = DEF_DEST_PORT;
	}

	// check there are enough parameters
	if (argc <= optind) {
		fprintf(stderr, "Missing host name.\n");
		exit(EXIT_FAILURE);
	}

	// if there is the 2nd paramater set packet length
	if (argc > optind+1) {
		opts.packet_len = atoi(argv[optind+1]);
		if (opts.packet_len > MAX_PACKET_LEN)
			opts.packet_len = MAX_PACKET_LEN;
	}
	rc = do_traceroute(argv[optind]);

	return rc;
}
