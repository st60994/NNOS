#include <stdio.h>		/* Basic I/O routines           */
#include <string.h>		/* memset, memmove, memcpy */
#include <stdlib.h>		/* exit */

#include <sys/types.h>		/* standard system types        */
#include <linux/types.h>	// required for linux/errqueue.h
#include <linux/errqueue.h>	// SO_EE_ORIGIN_ICMP
#include <netinet/ip_icmp.h>	// ICMP error message codes

#include <sys/time.h>		/* for timeout values           */

#include <errno.h>

/* getnameinfo, getaddrinfo */
// #include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>		// fcntl
#include <poll.h>		// poll


#include "options.h"
#include "ip_stuff.h"
#include "traceroute.h"

// #define	RESOLVE_TARGET_DOMAINNAME
// #undef	RESOLVE_TARGET_DOMAINNAME

#define HOST_UNKNOWN		"*"
#define HOST_UNKNOWN_BAD_AF	"*(bad AF)"

struct traceroute_info tr;

char *host_unknown = HOST_UNKNOWN;
char *host_unknown_bad_AF = HOST_UNKNOWN_BAD_AF;

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
	int pn;

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

#if PROBES_DYNAMIC == 1
	// allocate space for probes:
	tr.probes = calloc(opts.max_ttl * opts.probes, sizeof(struct probe_info));
	if (tr.probes == NULL)
		return 1;
#endif
	// init probes
	for (pn = 0; pn < opts.max_ttl * opts.probes; pn++) {
		tr.probes[pn].socket = -1;
	}

	return 0;
}

int traceroute_init_probe(int pn)
{
	return 0;
}

int traceroute_free_mem_probe(int pn)
{
	int pnttl;

	// free data
	free(tr.probes[pn].data);

	// hostname string exists && is not static
	if (tr.probes[pn].recv_hostname && tr.probes[pn].recv_hostname != host_unknown) {
		// check other probes on the same TTL level for the same pointer
		// and free mem only if it is not used
		for (pnttl = (pn/opts.probes)*opts.probes; pnttl < (pn/opts.probes+1)*opts.probes; pnttl++) {
			if (pnttl == pn)
				continue;
			if (tr.probes[pnttl].recv_hostname == tr.probes[pn].recv_hostname) {	// used somewhere else
				tr.probes[pn].recv_hostname = NULL;	// just set to NULL
				break;
			}
		}
		if (tr.probes[pn].recv_hostname) {	// it's not used elsewhere, free it
			free(tr.probes[pn].recv_hostname);
			tr.probes[pn].recv_hostname = NULL;
		}
	}

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

int traceroute_probe2ttl(int pn) {
	return (1 + (pn / opts.probes));
}

// inicializuje sockety???
// pošle packet s nastaveným TTL
int traceroute_send(int pn) {
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
	val = traceroute_probe2ttl(pn);
	if (setsockopt(tr.probes[pn].socket, SOL_IP, IP_TTL, &val, sizeof(val)) < 0) {
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

// set final probe so the probing can stop on the target
void set_final_probe(pn)
{
	// set final probe only if it's not yet set or
	// we reached the destination earlier then previous final probe
	if (tr.final_probe < 0 || pn < tr.final_probe) {
		DEBUGNL(" Setting the final probe: %d", pn);
		tr.final_probe = pn;
	}
	else
		DEBUGNL(" NOT setting the final probe: %d", pn);
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

	// we're done
	set_final_probe(pn);

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

	// poll = wait for a) timeout, or b) ICMP message, or c) answer
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
		// we're done
		set_final_probe(pn);
	}

	close(tr.probes[pn].socket);
	tr.probes[pn].socket = -1;

	return 0;
}

// get address of a host in newly allocated string
// how:
//	0 == noresolve (IP string)		ex.: 1.2.3.4
//	1 == resolve (hostname and IP)		ex.: foo.bar (1.2.3.4)
//	2 == resolve (hostname only)		ex.: foo.bar

char *addr2str(sockaddr_any *sa, int how)
{
	char *str;
	char ipstr[INET6_ADDRSTRLEN];
	char nameip[MAX_NAME_LEN];

	if (!sa->sa.sa_family)
		return host_unknown_bad_AF;

	// IP na string IP
	ipstr[0] = '\0';
	getnameinfo(&sa->sa, sizeof(*sa), ipstr, sizeof(ipstr), 0, 0, NI_NUMERICHOST);

	if (how == A2S_NORESOLVE)
		return strdup(ipstr);

	nameip[0] = '\0';
	getnameinfo(&sa->sa, sizeof(*sa), nameip, sizeof(nameip), 0, 0, NI_IDN);
	// foo on errors
	str = strdup(nameip);
	if (how == A2S_RESOLVE_H)	// just hostname
		return str;

	// hostname + IP (if resolved)
	if (strcmp(ipstr, nameip) != 0) {
		str = realloc(str, strlen(nameip) + strlen(ipstr) + 4); // 4 = " ()\0"
		str = strcat(str, " (");
		str = strcat(str, ipstr);
		str = strcat(str, ")");
	}
	return str;
}

// compare IPs
int same_ipaddr(sockaddr_any *sa1, sockaddr_any *sa2)
{
	if (sa1->sa.sa_family != sa2->sa.sa_family)
		return 0;
	switch (sa1->sa.sa_family) {
		case AF_INET:
			return sa1->sin.sin_addr.s_addr == sa2->sin.sin_addr.s_addr;
		case AF_INET6:
			return memcmp(sa1->sin6.sin6_addr.s6_addr, sa2->sin6.sin6_addr.s6_addr, sizeof(struct in6_addr)) == 0;
		default:
			return 0;
	}
}

// pošle jeden packet a zjistí výsledek
int do_one_probe(int pn)
{
	if (traceroute_init_probe(pn) != 0) {
		tr.probes[pn].probe_result = PROBE_ERR_INIT;
	}
	else if (traceroute_send(pn) != 0) {
		close(tr.probes[pn].socket);
		tr.probes[pn].socket = -1;
		tr.probes[pn].probe_result = PROBE_ERR_SEND;
	}
	else if ((traceroute_receive(pn, opts.timeout)) == 0) {
		tr.probes[pn].probe_result = PROBE_OK;
		// the 1st try on this TTL -> resolve hostname
		// other tries -> if the address is the same as previous, use it, otherwise resolve
		if (pn % opts.probes == 0
		    || !same_ipaddr(&tr.probes[pn-1].recv_addr, &tr.probes[pn].recv_addr)
		    || !tr.probes[pn-1].recv_hostname) {
			tr.probes[pn].recv_hostname = addr2str(&tr.probes[pn].recv_addr, !opts.noresolve);
		}
		else
			tr.probes[pn].recv_hostname = tr.probes[pn-1].recv_hostname;

		if (tr.probes[pn].err_str[0])
			tr.probes[pn].probe_result = PROBE_ERR_RECV;
	}
	else {
		close(tr.probes[pn].socket);
		tr.probes[pn].socket = -1;
		tr.probes[pn].probe_result = PROBE_ERR_TIMEOUT;
		tr.probes[pn].recv_hostname = host_unknown;
	}
	return tr.probes[pn].probe_result;;
}

int print_one_probe(int pn)
{
	switch (tr.probes[pn].probe_result) {
		case PROBE_ERR_INIT:	// malloc or something fatal
			exit(EXIT_FAILURE);
			return EXIT_FAILURE;
			// break;
		case PROBE_ERR_SEND:	// socket allocation, bind, connect or send
			exit(EXIT_FAILURE);
			return EXIT_FAILURE;
			// break;
		case PROBE_ERR_TIMEOUT:	// no answer
		case PROBE_OK:		// answer ok (ttl error, or dest. reached)
		case PROBE_ERR_RECV:	// like net, or host unreachable
			// print TTL
			if (pn % opts.probes == 0)
				printf("%2u.", traceroute_probe2ttl(pn));
			// print hostname
			if (tr.probes[pn].recv_hostname) {
			       if (pn % opts.probes == 0	// every TTL = new address to print
				   || tr.probes[pn].recv_hostname == host_unknown // * is printed always
				   || tr.probes[pn].recv_hostname == host_unknown_bad_AF // *
				   || (tr.probes[pn-1].recv_hostname != tr.probes[pn].recv_hostname // pointers differ
				       && strcmp(tr.probes[pn-1].recv_hostname, tr.probes[pn].recv_hostname))) // strings also differ
					printf(" %s", tr.probes[pn].recv_hostname);
			       // otherwise the name is the same as previous
			       // and there's no need to print it again
			}
			else
				printf(" *?"); // uninitialized? -- should NOT happen
			if (tr.probes[pn].recv_time) {
				double diff = tr.probes[pn].recv_time - tr.probes[pn].send_time;
				printf("  %.3f ms", diff * 1000);
			}
			if (tr.probes[pn].err_str[0])
				printf(" %s", tr.probes[pn].err_str);
			if (pn % opts.probes == opts.probes-1)
				printf("\n");
			fflush(stdout);
			break;
		default:
			printf("Unknown error?\n");
			fflush(stdout);
			break;
	}
	return 0;
}

