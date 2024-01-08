// Operating Systems: sample code  (c) Tomáš Hudec
// IPC, Message Queues
// System V:	msgget(2), msgctl(2), msgsnd(3), msgrcv(3)
// POSIX:	mq_open(3), mq_send(3), mq_close(3)

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
	struct msqid_ds mq_data;
	char *buffer;
	key_t mq_key;		// 32-bit signed int
	int i;

	// create the (unique) key from the program file and a project number (0--255)
	mq_key = ftok(argv[0], 62);

	// get the message queue id, create a new queue if it does not exist
	mq_id = msgget(mq_key, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (mq_id == -1) {
		perror("msgget failed");
		exit(EXIT_FAILURE);
	}
	// fprintf(stderr, "Created the message queue: key = %d, id = %d\n", mq_key, mq_id);
	printf("0x%x\n", mq_key);

	// get info about the message queue
	if (msgctl(mq_id, IPC_STAT, &mq_data) == -1) {
		perror("msgctl failed");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Maximum number of bytes in the message queue: %d\n",
		(int)(mq_data.msg_qbytes));

	msg_data.msg_type = 1;	// message type

	i = 1;
	do {
		// if no arguments: send the default message
		buffer = argc > 1 ? argv[i] : "no news is good news";
		// send each argument to the message queue
		strncpy(msg_data.msg_text, buffer, MSG_MAX);
		if (msgsnd(mq_id, (void *) &msg_data, strlen(buffer), 0) == -1) {
			perror("msgsnd failed");
			exit(EXIT_FAILURE);
		}
		++i;
	} while (i < argc);
		
	// not removing the queue

	exit(EXIT_SUCCESS);
}
