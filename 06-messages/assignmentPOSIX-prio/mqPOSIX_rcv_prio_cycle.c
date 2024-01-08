// Operating Systems: sample code  (c) Tomáš Hudec
// IPC, Message Queues
// mq_open(3), mq_receive(2), mq_getattr(3), mq_close(3), mq_unlink(3)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>	// S_IRUSR, S_IRWXU, …
#include <fcntl.h>

// IPC POSIX

char *mq_name = NULL;
char *message = NULL;

int main(int argc, char *argv[])
{
	int status;

	if (argc <= 1) {
		fprintf(stderr, "Expected a queue name as the first argument.\n");
		exit(EXIT_FAILURE);
	}
	mq_name = argv[1];

	status = 0;
	while (status < 2) {
			switch (errno) {
			case EAGAIN:	// no more messages in the queue
				if (status++)
					fprintf(stderr, "No more messages in the queue.\n");
				else {
					fprintf(stderr, "Waiting for more messages.\n");
					sleep(2);
				}
				break;
			default:	// another error
				perror("mq_receive failed");
				exit(EXIT_FAILURE);
			}
			continue;

		// print the received message

		status = 0;
	}

	exit(EXIT_SUCCESS);
}
