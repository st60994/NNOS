// Operating Systems: sample code  (c) Tomáš Hudec
// IPC, Message Queues
// msgget(2), msgctl(2), msgsnd(3), msgrcv(3)

#define MSG_MAX 30		// max message size

// message type for msgop(2)
struct msgbuf {
	long int msg_type;
	char msg_text[MSG_MAX];
};

