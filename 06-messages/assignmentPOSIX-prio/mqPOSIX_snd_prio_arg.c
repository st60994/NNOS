// Operating Systems: sample code  (c) Tomáš Hudec
// IPC, Message Queues
// mq_open(3), mq_send(3), mq_close(3), mq_unlink(3)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>	// S_IRUSR, S_IRWXU, …
#include <fcntl.h>

// IPC POSIX

#define min(a, b)	((a)<(b) ? (a) : (b))

int main(int argc, char *argv[])
{
	unsigned prio;
	int i;

	// test arguments
	if (argc <= 1) {
		fprintf(stderr, "Arguments needed: pairs priority and message.\n");
		exit(EXIT_FAILURE);
	}

	// print info
	fprintf(stderr, "Sending %d messages.\n", (argc-1)/2);

	for (i = 1; i+1 < argc; ++i) {
		// get priority
		errno = 0;
		prio = strtoul(argv[i++], NULL, 0);
		if (errno) {
			perror("strtoul failed");
			exit(EXIT_FAILURE);
		}

	}
		
	// print info
	fprintf(stderr, "All messages are sent.\n");

	exit(EXIT_SUCCESS);
}
