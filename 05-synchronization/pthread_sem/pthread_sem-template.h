// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections, Synchronization: mutex, condition variable
//
// Assignment:
// Design and implement a semaphore with counter
// using POSIX mutex and condition variable.
//
// Zadání:
// Navrhněte a implementujte semafor s čítačem
// pomocí posixového mutexu a podmínkové proměnné.
//
// Last modification: 2016-11-21

#include <errno.h>
#include <limits.h>			// UINT_MAX
#include <pthread.h>


// semaphore type
typedef struct {
	unsigned int	counter;	// semaphore counter
	unsigned int	nwaiters;	// number of waiters
	pthread_mutex_t	mutex;		// for mutual exclusion inside semaphore functions
	pthread_cond_t	cond;		// for signaling
} pt_sem_t;
#define PT_SEM_COUNTER_MAX	UINT_MAX

// initialize a semaphore
int pt_sem_init(pt_sem_t *sem, const unsigned int value);
// destroy a semaphore
int pt_sem_destroy(pt_sem_t *sem);
// the wait operation
int pt_sem_wait(pt_sem_t *sem);
// the signal operation
int pt_sem_post(pt_sem_t *sem);
// return values:
#define PT_SEM_OK		0	// 0: no error
#define PT_SEM_ERROR_MUTEX	1	// 1: error while working with mutex
#define PT_SEM_ERROR_COND	2	// 2: error while working with condition variable
#define PT_SEM_OVERFLOW		3	// 3: overflow of the counter

// implementation

// initialize a semaphore structure members
int pt_sem_init(pt_sem_t *sem, const unsigned int value)
{
	// FIXME
	return PT_SEM_OK;
}

// destroy a semaphore structure members
int pt_sem_destroy(pt_sem_t *sem)
{
	// FIXME
	return PT_SEM_OK;
}

// error codes for mutex unlock that are not checked:
//	ec	reason why it cannot occur
//	EINVAL	would fail already while locking
//	EPERM	thread owns the mutex because lock did not fail
int pt_sem_wait(pt_sem_t *sem)
{
	int rc = PT_SEM_OK;
	int err = errno;	// save errno

	// FIXME
	errno = err;		// restore/set errno
	return rc;
}

int pt_sem_post(pt_sem_t *sem)
{
	int rc = PT_SEM_OK;
	int err = errno;	// save errno

	// FIXME
	errno = err;		// restore/set errno
	return rc;
}

// EOF
