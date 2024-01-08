/*
 * simple traceroute
 *
 */

// #define _GNU_SOURCE		/* kvůli NI_IDN */
//
#include <stdio.h>		/* Basic I/O routines           */
#include <string.h>		/* memset, memmove, memcpy */
#include <stdlib.h>		/* exit */
#include <errno.h>

#include "options.h"
#include "ip_stuff.h"
#include "traceroute.h"

#include <locale.h>		// locales
/*
 * #include <sys/file.h>
 * #include <sys/ioctl.h>
 */

int do_traceroute(char *target_addr)
{
	char *resolved_addr = NULL;
	int pn;		// probe counter
	int rc;

	if (traceroute_init(target_addr, opts.af) != 0) {
		// error reporting done inside
		return 1;
	};
	printf("traceroute to ");
#ifdef RESOLVE_TARGET_DOMAINNAME
	// print resolved target hostname:
	resolved_addr = addr2str(&tr.dest_addr, !opts.noresolve);
	if (strcmp(target_addr,resolved_addr))
		printf("%s -> ", target_addr);
	printf("%s", resolved_addr);
#else
	// print given targed and its IP
	resolved_addr = addr2str(&tr.dest_addr, A2S_NORESOLVE);
	if (strcmp(target_addr, resolved_addr))
		printf("%s (%s)", target_addr, resolved_addr);
	else
		printf("%s", target_addr);
#endif
	// other info
	printf(" port %d%s (max ttl %d, %d probes per ttl, packet size %d bytes):\n",
		opts.dest_port, opts.dest_port_seq ? "+" : "",
		opts.max_ttl, opts.probes,
		opts.packet_len);
	// do probes
	for (pn = 0; pn < opts.max_ttl * opts.probes; pn++) {
		rc = do_one_probe(pn);
		rc = print_one_probe(pn);
		if (tr.final_probe >= 0 && pn % opts.probes == opts.probes-1) {
			// we're done
			break;
		}
	}
	// free mem
	if (resolved_addr != host_unknown && resolved_addr != host_unknown_bad_AF)
		free(resolved_addr);
	for (pn = 0; pn <= tr.final_probe; pn++) {
		traceroute_free_mem_probe(pn);
	}
#if PROBES_DYNAMIC == 1
	free(tr.probes);
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	int rc;			// return value storage

	setlocale (LC_ALL, "");

	// parse options
	parse_options(argc, argv);

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
