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
#include <stdatomic.h>					// atomic_flag
#include <sched.h>						// sched_yield
#include <pthread.h>					// mutex
#include <errno.h>						// errno
#include <sys/stat.h>					// S_IRUSR, S_IWUSR
#include <fcntl.h>						// O_CREAT, O_EXCL
#include <semaphore.h>					// semaphores
#include <sys/sem.h>					// SETVAL, SEM_UNDO
#include <sys/ipc.h>					// IPC_RMID, IPC_PRIVATE
#include <mqueue.h>						// mqd_t
#include <sys/msg.h>					// SysV message queue

bool busy_wait_yields = false;			// set by the main program

// macros, variable declarations and function definitions for critical section access control

volatile bool locked;						//variable for LOCKED and TEST_XCHNG methods

volatile atomic_flag locked_atomic;			// variable with atomic flag for TEST_XCHNG method

pthread_mutex_t cs_mutex;

#define SEM_NAME	"/named_sem_st61014"		// defined name for semaphore
sem_t* cs_named_sem;							// variable to contain the name semaphore

sem_t cs_sem;				// variable to contain semaphore

union semunion {			// define semunion, to have its size known
	int val;				// semaphore value for SETVAL
	struct semid_ds* buf;	// buffer for IPC_STAT and IPC_SET
	unsigned short* array;	// array for SETALL and GETALL
};

struct sembuf sem_op_enter;			// struct containing what operations should be performed during cs_enter

struct sembuf sem_op_leave;			// struct containing what operations should be performed during cs_leave

int sysv_semid;				// variable to store semaphore identifier after semget()

mqd_t message_queue;				// variable to store message queue descriptor

#define MQ_NAME "/st61014_mq"			// name for message queue, contains number from st61014
#define MESSAGE_SIZE 10					// message size is 10 bytes
#define MESSAGE "Message"				// message text

char message_buffer[MESSAGE_SIZE + 1];			// message buffer to store received message data in, MESSAGE_SIZE+1 to include end of line symbol

struct mq_attr mq_attributes;					// struct to store attributes and then pass it to mq_open()

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
		locked = false;			// sets locked to false for both CS_METHOD_LOCKED and CS_METHOD_TEST_XCHG
		break;
	case CS_METHOD_XCHG:
		atomic_flag_clear(&locked_atomic);		// initializing without use of ATOMIC_FLAG_INIT
		break;
	case CS_METHOD_MUTEX:
		if ((errno = pthread_mutex_init(&cs_mutex, NULL))) {			// initializes mutex with default values (default is set by NULL)
			perror("pthread_mutex_init failed");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX:
		if (sem_init(&cs_sem, 0, 1)) {			// initializes semaphore, 0 to share semaphore between threads of a process, 1 as initial value of semaphore
			perror("sem_init failed");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		if (SEM_FAILED == (cs_named_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1))) {			
																		// opens semaphore with name defined in SEM_NAME
																		// O_CREAT creates semaphore if it does not exist
																		// because O_CREAT was specified added O_EXCL, fail with error if SEM_NAME already exists
																		// S_IRUSR sets read permission to owner, S_IWUSR sets write permission to owner
																		// 1 as initial value of the semaphore
			perror("sem_open");
			exit(EXIT_FAILURE);
		}
		sem_unlink(SEM_NAME);		// remove the semaphore after closing
		break;
	case CS_METHOD_SEM_SYSV:
		sem_op_enter.sem_num = 0;				// semaphore index, 0 because we only have 1 semaphore
		sem_op_enter.sem_op = -1;				// semaphore operation to do, -1 to lower semaphore counter 
		sem_op_enter.sem_flg = SEM_UNDO;		// set sempahore flag to SEM_UNDO, revert operation after semaphore is terminated

		sem_op_leave.sem_num = 0;				// semaphore index, 0 because we only have 1 semaphore
		sem_op_leave.sem_op = 1;				// semaphore operation to do, 1 to increase semaphore counter 
		sem_op_leave.sem_flg = SEM_UNDO;		// set sempahore flag to SEM_UNDO, revert operation after semaphore is terminated

		sysv_semid = semget(IPC_PRIVATE, 1, 0600);					// create semaphore and save its ID
																// 1 represents number of semaphores
																// IPC_PRIVATE to create semaphore belonging to the process
																// 0600 is to set read and write permissions for user
		if (sysv_semid == -1) {
			perror("Error creating semaphore");
			exit(EXIT_FAILURE);
		}
		union semunion arg;			// union containing arguments that are passed to semaphore with semctl
		arg.val = 1;				// set initial value to 1 for semaphore counter
		if (semctl(sysv_semid, 0, SETVAL, arg) == -1) {			// performs specified operation on semaphore 
															// 0 represents the index of semaphore
															// SETVAL flag to set value contained in arg on semaphore
			perror("Error setting semaphore value");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_POSIX:
		mq_attributes.mq_flags = 0;					// sets the message queue to be blocking
		mq_attributes.mq_maxmsg = 1;				// sets maximum number of messages in queue to 1
		mq_attributes.mq_msgsize = MESSAGE_SIZE;	// sets message size to defined value MESSAGE_SIZE
		mq_attributes.mq_curmsgs = 0;				// sets number of current messages in queue to 0
		if ((message_queue = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_RDWR, S_IWUSR | S_IRUSR, &mq_attributes)) == (mqd_t)-1) {
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
		if (mq_unlink(MQ_NAME) == -1) {			// unlink the queue so its only kept while the process is running
			perror("mq_unlink");
			exit(EXIT_FAILURE);
		}
		if (mq_send(message_queue, MESSAGE, MESSAGE_SIZE, 0) == -1) {			// send first initial message, 0 represents priority
			perror("mq_send");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_SYSV:
		msg_buf_sysv.msg_type = 1;
		if ((mq_sysv = msgget(IPC_PRIVATE, 0600)) == -1) {			// gets message queue identifier
																	// IPC_PRIVATE to create mq belonging to the process
																	// 0600 adds read and write permissions only to owner
			perror("msgget");
			exit(EXIT_FAILURE);
		}
		if ((msgsnd(mq_sysv, (void*)&msg_buf_sysv, 0, 0) == -1)) {			// sends innitial message to queue
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
		if ((errno = pthread_mutex_destroy(&cs_mutex))) {			// destroys the mutex
			perror("pthread_mutex_destroy");
		}
		break;
	case CS_METHOD_SEM_POSIX:
		if (sem_destroy(&cs_sem)) {					// destoys the semaphore
			perror("sem_destroy");
		}
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		if (SEM_FAILED != cs_named_sem) {			// verify cs_name_sem contains semaphore and not SEM_FAILED
			if (sem_close(cs_named_sem)) {			// close the semaphore
				perror("sem_close");
			}
			cs_named_sem = SEM_FAILED;				// overwrite saved semaphore to ensure no links to it remain
		}
		break;
	case CS_METHOD_SEM_SYSV:
		if (semctl(sysv_semid, 0, IPC_RMID) == -1) {			// removes the semaphore with specified semid and index 0
														// IPC_RMID flag to remove semaphore, semaphore ceases to exist when the process ends
			perror("Error destroying semaphore");
		}
		break;
	case CS_METHOD_MQ_POSIX:
		if (mq_close(message_queue) == -1) {			// close the queue
			perror("mq_close");
		}
		break;
	case CS_METHOD_MQ_SYSV:
		if (msgctl(mq_sysv, IPC_RMID, NULL) == -1) {			// remove the message queue
																// IPC_RMID flag to remove the message queue
																// NULL because we do not pass any msqid_ds struct 
			perror("msgctl_destroy");
		}
		break;
	}
	cs_var_allocated = false;
}

// before entering the critical section
void cs_enter(int id)
{
	switch (cs_method_used) {
	case CS_METHOD_ATOMIC:
		break;
	case CS_METHOD_LOCKED:
		while (locked) {			// if locked just wait here
			if (busy_wait_yields) {
				sched_yield();		// relinquish the CPU
			}
		}
		locked = true;
		break;
	case CS_METHOD_TEST_XCHG:
		while (atomic_load_explicit(&locked, memory_order_relaxed) || atomic_exchange_explicit(&locked, true, memory_order_acquire)) {
									// test and test and set, setting locked to true
									// memory_order_relaxed sets no synchronization or ordering constraint, only atomicity is required of this operation
									// memory_order_acquire sets that no read or writes in the current thread are done before this one
			if (busy_wait_yields) {
				sched_yield();		// relinquish the CPU
			}
		}
		break;
	case CS_METHOD_XCHG:
		while (atomic_flag_test_and_set(&locked_atomic)) {			// test and set with atomic locked variable
			if (busy_wait_yields) {
				sched_yield();		// relinquish the CPU
			}
		}
		break;
	case CS_METHOD_MUTEX:
		pthread_mutex_lock(&cs_mutex);			// attempts to lock the mutex
												// not error checking according to assignment
		break;
	case CS_METHOD_SEM_POSIX:
		sem_wait(&cs_sem);			// decrements semaphore counter if semaphore value is greater then zero, else it waits until the decrement can happen
									// not error checking according to assignment
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		sem_wait(cs_named_sem);			// decrements semaphore counter if semaphore value is greater then zero, else it waits until the decrement can happen
										// not error checking according to assignment
		break;
	case CS_METHOD_SEM_SYSV:
		semop(sysv_semid, &sem_op_enter, 1);			// 1 represents the number of semaphores
												// not error checking according to assignment
		break;
	case CS_METHOD_MQ_POSIX:
		mq_receive(message_queue, message_buffer, MESSAGE_SIZE, NULL);			// try to receive a message, because its blocking it will wait here until a message is available
																				// received data is saved to message_buffer
																				// NULL because we do not require priority
																				// not error checking according to assignment
		break;
	case CS_METHOD_MQ_SYSV:
		msgrcv(mq_sysv, (void*)&msg_buf_sysv, 0, 0, 0);			// receive message from queue
																// we can use same msg_buf_sysv because content of all messages is the same
																// message size is 0 because we send them with size 0
																// message type is 0 to take out first message in queue
																// message flags are 0 because we do not use flags
																// not error checking according to assignment
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
		locked = false;
		break;
	case CS_METHOD_TEST_XCHG:
		atomic_store_explicit(&locked, false, memory_order_release);			// set locked as false and release the set memory order
																				// memory_order_release ensures all memory accesses are ordered properly around atomic operation
		break;
	case CS_METHOD_XCHG:
		atomic_flag_clear(&locked_atomic);		// clear the atomic flag
		break;
	case CS_METHOD_MUTEX:
		pthread_mutex_unlock(&cs_mutex);		// attempts to unlock the mutex
												// not error checking according to assignment
		break;
	case CS_METHOD_SEM_POSIX:
		sem_post(&cs_sem);						// increments semaphore counter, if semaphore value consequently becomes greater than zero another thread waiting will be woken up
												// not error checking according to assignment
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		sem_post(cs_named_sem);					// increments semaphore counter, if semaphore value consequently becomes greater than zero another thread waiting will be woken up
												// not error checking according to assignment
		break;
	case CS_METHOD_SEM_SYSV:
		semop(sysv_semid, &sem_op_leave, 1);			// 1 represents the number of semaphores
												// not error checking according to assignment
	
		break;
	case CS_METHOD_MQ_POSIX:
		mq_send(message_queue, MESSAGE, MESSAGE_SIZE, 0);			// send message to queue again with priority 0
																	// not error checking according to assignment

		break;
	case CS_METHOD_MQ_SYSV:
		msgsnd(mq_sysv, (void*)&msg_buf_sysv, 0, 0);			// sends message to queue
																// msg_buf_sysv contains message params
																// message size is 0
																// we do not use any flags, so 0
																// not error checking according to assignment

		break;
	}
}

// vim:ts=4:sw=4
// EOF
