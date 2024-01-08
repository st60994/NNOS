#include <stdio.h>		// stdin, stdout, stderr
#include <stdlib.h>		// exit

#include "options.h"

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
	.debug =		0,
	.verbose =		1,
};

void usage(FILE *f)
{
	fprintf(f,
	"Use:\n"
	"  %s [-hVvQd] [-f first_TTL] [-m max_TTL] [-n] [-w wait_timeout] "
	"[-q queries_per_TTL] [-U] [-p dst_port] "
	"host [packet_length]\n"
	"Options:\n"
	"  -h            This help.\n"
	"  -v            More verbosity.\n"
	"  -Q            Less verbosity.\n"
	"  -d            Debug mode on. Prints some debug messages.\n"
	"  -f first_TTL  Start from the first_ttl hop (instead of " _TEXT(FIRST_TTL) ").\n"
	"  -m max_TTL    Set max. TTL to be reached. Default is " _TEXT(DEF_TTL) ". Max. is " _TEXT(MAX_TTL) ".\n"
	"  -n            Do not resolve IP addresses to domain names.\n"
	"  -w timeout    Set waiting time in seconds for a response. Default is %.1f s.\n"
	"  -q queries    Set the number of probes per each hop. Default is " _TEXT(DEF_PROBES) ". Max. is " _TEXT(MAX_PROBES) ".\n"
	"  -U            Use specific UDP destination port for tracerouting. See -p.\n"
	"  -p dst_port   Set UDP destination port (with -U, default is " _TEXT(DEF_DEST_PORT) ") or port base (without -U, default is " _TEXT(DEF_DEST_PORT_BASE) ").\n"
	"Arguments:\n"
	"  host          Hostname or IP address.\n"
	"  packet_length Length of a packet to send. Default is " _TEXT(DEF_PACKET_SIZE) ".\n"
	"Error strings:\n"
	"  !N      network unreachable or unknown\n"
	"  !H      host unreachable or unknown\n"
	"  !X      network or host prohibited (filtered or firewalled)\n"
	"  !P      protocol unreachable\n"
	"  !F      fragmentation needed (try smaller packets)\n"
	"  !S      source route failed\n"
	"  !V      host precedence violation\n"
	"  !C      precedence cutoff\n"
	"  !<n>    destination unreachable (other ICMP error code)\n"
	"  !<m-n>  ICMP error type other than destination unreachable (m), code n\n"
	"",
	ME, DEF_TIMEOUT);
}

// parse options
void parse_options(int argc, char *argv[])
{
	int opt;		// options

	while ((opt = getopt(argc, argv, "hVvQdf:m:nw:q:Up:")) != -1) {
		switch (opt) {
			case 'h':
				usage(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
				printf("%s version %s\n", ME, VERSION);
				exit(EXIT_SUCCESS);
			case 'v':
				if (opts.verbose < 100)
					opts.verbose++;
				break;
			case 'Q':
				if (opts.verbose > 0)
					opts.verbose--;
				break;
			case 'd':
				opts.debug = 1;
				break;
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

}

