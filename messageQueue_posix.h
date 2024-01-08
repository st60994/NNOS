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
#include <mqueue.h>						// mqd_t
#include <sys/stat.h>					// S_IRUSR, S_IWUSR
#include <fcntl.h>						// O_CREAT, O_EXCL
#include <errno.h>						// errno


bool busy_wait_yields = false;			// set by the main program

// macros, variable declarations and function definitions for critical section access control

mqd_t message_queue;				// variable to store message queue descriptor

#define MQ_NAME "/st61014_mq"			// name for message queue, contains number from st61014
#define MESSAGE_SIZE 10					// message size is 10 bytes
#define MESSAGE "Message"				// message text

char message_buffer[MESSAGE_SIZE+1];			// message buffer to store received message data in, MESSAGE_SIZE+1 to include end of line symbol

struct mq_attr mq_attributes;					// struct to store attributes and then pass it to mq_open()

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
		mq_attributes.mq_flags = 0;					// sets the message queue to be blocking
		mq_attributes.mq_maxmsg = 1;				// sets maximum number of messages in queue to 1
		mq_attributes.mq_msgsize = MESSAGE_SIZE;	// sets message size to defined value MESSAGE_SIZE
		mq_attributes.mq_curmsgs = 0;				// sets number of current messages in queue to 0
		if ((message_queue = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_RDWR, S_IWUSR | S_IRUSR, &mq_attributes)) == (mqd_t) -1) {
															// open the queue
															// needs to be verified against (mqd_t) -1 because it returns value of type mqd_t
															// MQ_NAME is the name of the queue
															// O_CREAT and O_RDWR creates the queue with read and write permissions
															// because O_CREAT was specified added O_EXCL, fail with error if MQ_NAME already exists
															// S_IWUSR and S_IRUSR gives the owner user read and write permissions
															// mq_attributes specifies flags and limits set above
			perror("mq_open");
			exit(EXIT_FAILURE);
		}
		if (mq_send(message_queue, MESSAGE, MESSAGE_SIZE, 0) == -1) {			// send first initial message, 0 represents priority
			perror("innit_mq_send");
			exit(EXIT_FAILURE);
		}
		if (mq_unlink(MQ_NAME) == -1) {			// unlink the queue so its only kept while the process is running
			perror("mq_unlink");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_SYSV:
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
		if (mq_close(message_queue) == -1) {			// close the queue
			perror("mq_close");
		}
		else {
			cs_var_allocated = false;
		}
		break;
	case CS_METHOD_MQ_SYSV:
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
		if (mq_receive(message_queue, message_buffer, MESSAGE_SIZE, NULL) == -1) {			// try to receive a message, because its blocking it will wait here until a message is available
																							// received data is saved to message_buffer
																							// NULL because we do not require priority
			perror("mq_receive");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_SYSV:
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
		if (mq_send(message_queue, MESSAGE, MESSAGE_SIZE, 0) == -1) {			// send message to queue again with priority 0
			perror("mq_send");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// vim:ts=4:sw=4
// EOF
