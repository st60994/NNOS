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
#include <pthread.h>					// mutex
#include <errno.h>						// errno
#include <sys/stat.h>					// S_IRUSR, S_IWUSR
#include <fcntl.h>						// O_CREAT
#include <semaphore.h>					// semaphores


bool busy_wait_yields = false;			// set by the main program

// macros, variable declarations and function definitions for critical section access control

pthread_mutex_t cs_mutex;

#define SEM_NAME	"/named_sem_st61014"		// defined name for semaphore
sem_t* cs_named_sem;			// variable to contain the name semaphore

sem_t cs_sem;			// variable to contain semaphore

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
		if (SEM_FAILED == (cs_named_sem = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 1))) {			// opens semaphore with name defined in SEM_NAME
																										// O_CREAT creates semaphore if it does not exist
																										// S_IRUSR sets read permission to owner, S_IWUSR sets write permission to owner
																										// 1 as initial value of the semaphore
			perror("sem_open");
			exit(EXIT_FAILURE);
		}
		sem_unlink(SEM_NAME);		// remove the semaphore after closing
		break;
	case CS_METHOD_SEM_SYSV:
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
		if ((errno = pthread_mutex_destroy(&cs_mutex))) {			// destroys the mutex
			perror("pthread_mutex_destroy");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX:
		if (sem_destroy(&cs_sem)) {			// destoys the semaphore
			perror("sem_destroy");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		if (SEM_FAILED != cs_named_sem) {			// verify cs_name_sem contains semaphore and not SEM_FAILED
			if (sem_close(cs_named_sem)) {			// close the semaphore
				perror("sem_close");
				exit(EXIT_FAILURE);
			}
			cs_named_sem = SEM_FAILED;			// overwrite saved semaphore to ensure no links to it remain
		}
		break;
	case CS_METHOD_SEM_SYSV:
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
		if ((errno = pthread_mutex_lock(&cs_mutex))) {			// attempts to lock the mutex
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX:
		if (sem_wait(&cs_sem)) {			// decrements semaphore counter if semaphore value is greater then zero, else it waits until the decrement can happen
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		if (sem_wait(cs_named_sem)) {			// decrements semaphore counter if semaphore value is greater then zero, else it waits until the decrement can happen
			perror("sem_named_wait");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_SYSV:
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
		if ((errno = pthread_mutex_unlock(&cs_mutex))) {			// attempts to unlock the mutex
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX:
		if (sem_post(&cs_sem)) {			// increments semaphore counter, if semaphore value consequently becomes greater than zero another thread waiting will be woken up
			perror("sem_post");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		if (sem_post(cs_named_sem)) {			// increments semaphore counter, if semaphore value consequently becomes greater than zero another thread waiting will be woken up
			perror("sem_named_post");
			exit(EXIT_FAILURE);
		}
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// vim:ts=4:sw=4
// EOF
