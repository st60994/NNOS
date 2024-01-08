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
#include <sys/sem.h>					// SETVAL, SEM_UNDO
#include <sys/ipc.h>					// IPC_CREAT, IPC_RMID
#include <sys/types.h>					// key_t
#include <errno.h>						// errno


bool busy_wait_yields = false;			// set by the main program

// macros, variable declarations and function definitions for critical section access control

union semunion {			// define semunion, to have its size known
	int val;				// semaphore value for SETVAL
	struct semid_ds* buf;	// buffer for IPC_STAT and IPC_SET
	unsigned short* array;	// array for SETALL and GETALL
};

key_t sem_key;			// define key for semaphore

int semid;				// variable to store semaphore identifier after semget()

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
		sem_key = 61014;			// set key as number from st61014
		semid = semget(sem_key, 1, IPC_CREAT | 0600);			// create semaphore and save its ID
																// 1 represents number of semaphores
																// IPC_CREAT flag to create semaphore
																// 0600 is to set read and write permissions for user
		if (semid == -1) {
			perror("Error creating semaphore");
			exit(EXIT_FAILURE);
		}
		union semunion arg;			// union containing arguments that are passed to semaphore with semctl
		arg.val = 1;			// set initial value to 1 for semaphore counter
		if (semctl(semid, 0, SETVAL, arg) == -1) {			// performs specified operation on semaphore 
															// 0 represents the index of semaphore
															// SETVAL flag to set value contained in arg on semaphore
			perror("Error setting semaphore value");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_POSIX:
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
		if (semctl(semid, 0, IPC_RMID) == -1) {			// removes the semaphore with specified semid and index 0
														// IPC_RMID flag to remove semaphore, semaphore ceases to exist when the process ends
			perror("Error destroying semaphore");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_POSIX:
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
		struct sembuf sem_op_enter;			// struct containing what operations should be performed 

		sem_op_enter.sem_num = 0;			// semaphore index, 0 because we only have 1 semaphore
		sem_op_enter.sem_op = -1;			// semaphore operation to do, -1 to lower semaphore counter 
		sem_op_enter.sem_flg = SEM_UNDO;	// set sempahore flag to SEM_UNDO, revert operation after semaphore is terminated

		if (semop(semid, &sem_op_enter, 1) == -1) {			// 1 represents the number of semaphores
			perror("Error entering critical section");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_MQ_POSIX:
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
		struct sembuf sem_op_leave;			// struct containing what operations should be performed 

		sem_op_leave.sem_num = 0;			// semaphore index, 0 because we only have 1 semaphore
		sem_op_leave.sem_op = 1;			// semaphore operation to do, 1 to increase semaphore counter 
		sem_op_leave.sem_flg = SEM_UNDO;	// set sempahore flag to SEM_UNDO, revert operation after semaphore is terminated

		if (semop(semid, &sem_op_leave, 1) == -1) {			// 1 represents the number of semaphores
			perror("Error exiting critical section");
			exit(EXIT_FAILURE);
		}
		break;
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// vim:ts=4:sw=4
// EOF
