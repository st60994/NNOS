// Operating Systems: sample code  (c) Tomáš Hudec
// IPC, Message Queues
// msgget(2), msgctl(2), msgsnd(3), msgrcv(3)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>	// S_IRUSR, S_IRWXU, …
#include <sys/wait.h>	// wait(2)

// IPC System V
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "messagesSV.h"	// defines message size MSG_MAX and message data structure

int main(int argc, char *argv[])
{
	int mq_id;
	struct msgbuf msg_data;
	char buffer[MSG_MAX+1];
	key_t mq_key;		// 32-bit signed int
	ssize_t msg_size;

	if (argc <= 1) {
		fprintf(stderr, "Expected the key as the first argument.\n");
		exit(EXIT_FAILURE);
	}
	mq_key = (key_t) strtol(argv[1], NULL, 0);

	// get the message queue id
	mq_id = msgget(mq_key, 0);
	if (mq_id == -1) {
		perror("msgget failed");
		exit(EXIT_FAILURE);
	}
	printf("child: Opened the message queue: key = 0x%x, id = %d\n", mq_key, mq_id);

	fprintf(stderr, "child: Awaiting the message...\n");
	while (1) {
		// blocking receive -- wait for the message
		// message type == 0 -> accept any type
		// MSG_NOERROR = if the message is longer than given limit
		// the call does not fail but clips the message instead
		if ((msg_size = msgrcv(mq_id, (void *) &msg_data, MSG_MAX,
			0, MSG_NOERROR)) == -1) {
			perror("msgrcv failed");
			exit(EXIT_FAILURE);
		}
		else {
			strncpy(buffer, msg_data.msg_text, msg_size);
			buffer[msg_size] = '\0';
			printf("The child got the message: %s\n", buffer);
			exit(EXIT_SUCCESS);
		}
	}

	exit(EXIT_SUCCESS);
}
