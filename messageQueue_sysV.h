// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Section Access Control
// header file

// Modified: 2017-11-30, 2017-12-06, 2020-11-25, 2020-12-10, 2023-11-23

#define CS_METHOD_ATOMIC				0
#define CS_METHOD_LOCKED				1
#define CS_METHOD_XCHG					2
#define CS_METHOD_TEST_XCHG				3
#define CS_METHOD_MUTEX					4
#define CS_METHOD_SEM_POSIX				5
#define CS_METHOD_SEM_POSIX_NAMED		6
#define CS_METHOD_SEM_SYSV				7
#define CS_METHOD_MQ_POSIX				8
#define CS_METHOD_MQ_SYSV				9

#define CS_METHOD_MIN					CS_METHOD_LOCKED
#define CS_METHOD_MAX					CS_METHOD_MQ_SYSV
#define CS_METHODS_BUSY_WAIT			3

#include <stdbool.h>					// bool
#include <stdlib.h>						// exit
#include <stdio.h>						// fprintf
// additional includes for critical section access control methods
#include <sys/msg.h>					// SysV message queue
#include <sys/ipc.h>					// IPC_PRIVATE
#include <errno.h>						// errno


bool busy_wait_yields = false;			// set by the main program

// macros, variable declarations and function definitions for critical section access control
int mq_sysv;			// variable for SysV message queue identifier

struct msgbuf {			// struct message buffer for message params
	long int msg_type;
};

struct msgbuf msg_buf_sysv;			// message buffer


// note: inline is not used unless asked for optimization
#define FORCE_INLINE	__attribute__ ((always_inline)) static inline

// allocate/initialize variables used for the critical section access control
FORCE_INLINE
void cs_init(int method);

// destroy allocated variables used for the critical section access control
FORCE_INLINE
void cs_destroy(void);

// before entering the critical section
FORCE_INLINE
void cs_enter(int id);

// after leaving the critical section
FORCE_INLINE
void cs_leave(int id);


static int cs_method_used = -1;			// method used, initialized in cs_init()
static bool cs_var_allocated = false;	// successful allocation of variables


// implementation (the funcions are to be inlined, we need them here)


// allocate/initialize variables used for the critical section access control
void cs_init(int method)
{
	cs_method_used = method;
	switch (cs_method_used) {
	case CS_METHOD_ATOMIC:
		break;
	case CS_METHOD_LOCKED:
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		msg_buf_sysv.msg_type = 1;
		if ((mq_sysv = msgget(IPC_PRIVATE, 0600)) == -1) {			// gets message queue identifier
																	// IPC_PRIVATE to create mq belonging to the process
																	// 0600 adds read and write permissions only to owner
			perror("msgget");
			exit(EXIT_FAILURE);
		}
		if ((msgsnd(mq_sysv, (void*) &msg_buf_sysv,0,0) == -1)) {			// sends innitial message to queue
																			// msg_buf_sysv contains message params
																			// message size is 0
																			// we do not use any flags, so 0
			perror("innit_msgsnd");
			exit(EXIT_FAILURE);
		}
		break;
	default:
		fprintf(stderr, "Error: The method %d is not defined.\n", cs_method_used);
		exit(EXIT_FAILURE);
	}
	cs_var_allocated = true;
}

// destroy allocated variables used for the critical section access control
void cs_destroy(void)
{
	if (!cs_var_allocated)				// if not allocated: nothing to do
		return;
	switch (cs_method_used) {
	case CS_METHOD_ATOMIC:
	case CS_METHOD_LOCKED:
	case CS_METHOD_TEST_XCHG:
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		if (msgctl(mq_sysv, IPC_RMID, NULL) == -1) {			// remove the message queue
																// IPC_RMID flag to remove the message queue
																// NULL because we do not pass any msqid_ds struct 
			perror("msgctl_destroy");
		}
		else {
			cs_var_allocated = false;
		}
		break;
	}
}

// before entering the critical section
void cs_enter(int id)
{
	switch (cs_method_used) {
	case CS_METHOD_ATOMIC:
		break;
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		if (msgrcv(mq_sysv, (void*) &msg_buf_sysv, 0, 0, 0) == -1) {			// receive message from queue
																				// we can use same msg_buf_sysv because content of all messages is the same
																				// message size is 0 because we send them with size 0
																				// message type is 0 to take out first message in queue
																				// message flags are 0 because we do not use flags
			perror("msgrcv");
			exit(EXIT_FAILURE);
		}
		break;
	}
}

// after leaving the critical section
void cs_leave(int id)
{
	switch (cs_method_used) {
	case CS_METHOD_ATOMIC:
		break;
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		if ((msgsnd(mq_sysv, (void*) &msg_buf_sysv, 0, 0) == -1)) {			// sends message to queue
																			// msg_buf_sysv contains message params
																			// message size is 0
																			// we do not use any flags, so 0
			perror("msgsnd");
				exit(EXIT_FAILURE);
		}
		break;
	}
}

// vim:ts=4:sw=4
// EOF
