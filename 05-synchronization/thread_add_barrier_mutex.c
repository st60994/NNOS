// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections, Synchronization – Barrier
// POSIX mutexes:
// pthread_mutex_lock(3), pthread_mutex_unlock(3), pthread_mutex_init(3), pthread_mutex_destroy(3)
// POSIX barriers:
// pthread_barrier_init(3), pthread_barrier_wait(3), pthread_barrier_destroy(3)
// atexit(3)
//
// Modified: 2015-11-11, 2016-11-14, 2017-04-18, 2023-04-04

// A barrier is a tool that allows simultaneous execution of threads from a given point.

// We need to define _XOPEN_SOURCE >= 600 for portability (to access barriers):
#if !defined _XOPEN_SOURCE || _XOPEN_SOURCE < 600
#	define _XOPEN_SOURCE 600	// enable barriers
#endif

// Alternatively, we can define _POSIX_C_SOURCE:
//#if !defined _POSIX_C_SOURCE || _POSIX_C_SOURCE < 200112L
//	#define _POSIX_C_SOURCE 200112L	// this also enables barriers
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#define THREADS 2		// the number of threads
#define ITERATIONS 1000000	// the number of operations per thread

volatile int count = 0;		// shared variable

// declare a mutex 		// use static initialization
pthread_mutex_t mutex_count; 	// = PTHREAD_MUTEX_INITIALIZER;
bool mutex_count_initialized = false;	// set to true for static init

// a barrier declaration
pthread_barrier_t sync_start_threads;
bool sync_start_threads_barrier_initialized = false;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (mutex_count_initialized) {
		if ((errno = pthread_mutex_destroy(&mutex_count)))	// pthread functions are not required to set errno
			perror("pthread_mutex_destroy");
			// possible errors:			– why it should not happen:
			//	EINVAL (not a valid mutex)	– mutex is valid and initialized
			//	EBUSY (mutex is locked)		– we never exit while the mutex blocks
		else
			mutex_count_initialized = false;
	}
	if (sync_start_threads_barrier_initialized) {
		// release system resources allocated by the barrier
		if ((errno = pthread_barrier_destroy(&sync_start_threads)))
			perror("pthread_barrier_destroy");
		else
			sync_start_threads_barrier_initialized = false;
 	}
}

// synchronization by a barrier / synchronizace bariérou
static void synchronize_threads_start(void)
{
	switch ((errno = pthread_barrier_wait(&sync_start_threads))) {	// wait at the barrier and check the return code
		case PTHREAD_BARRIER_SERIAL_THREAD:
			// this will be executed only once by one (unspecified) thread
			printf("All threads are synchronized.\n");
			// we can also release shared resources that are not needed anymore (like the barrier itself)
		case 0:	// this will be executed by all threads
			break;
		default:// error
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
	}
}

void *ThreadAdd(void *arg)
{
	int i;
	// register int reg;

	synchronize_threads_start();		// synchronize threads start / synchronizace startu vláken

	for (i = 0; i < ITERATIONS; ++i) {
		// ENTRY SECTION
		pthread_mutex_lock(&mutex_count);	// like semaphore wait
		// return value is not checked, possible errors that should not happen:
		//	EINVAL (PTHREAD_PRIO_PROTECT used, thread's priority is higher)
		//	EINVAL (not a valid mutex)
		//	EAGAIN (recursive locks exceeded)
		//	EDEADLK (current thread owns the mutex)
		// CRITICAL SECTION
		// Compilation of "count++" is platform (CPU) dependent because
		// the CPU can store the value into register, increase that register
		// and store the result back into the memory variable.
		// We can simulate this behavior by three commands with a local variable.
		count++;		// usually compiled using a CPU register:
		// reg = count;		// save the global count locally
		// reg += 1;		// increment the local copy
		// count = reg;		// store the local value into the global one
		// You can check dissassembly using objdump:
		//	objdump -M intel --disassemble=ThreadAdd thread_add
		// or
		//	gdb -batch -ex 'disassemble/m ThreadAdd' thread_add
		// EXIT SECTION
		pthread_mutex_unlock(&mutex_count);	// like semaphore signal
		// return value is not checked, possible errors that should not happen:
		//	EINVAL (not a valid mutex)
		//	EAGAIN (recursive locks exceeded)
		//	EPERM (current thread does not own the mutex)
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid[THREADS];
	int t;

	atexit(release_resources);	// release resources at program exit

	// initialization of the mutex during run-time, NULL for no attributes
	if ((errno = pthread_mutex_init(&mutex_count, NULL))) {
		// possible errors:
		//	EINVAL (attr is invalid)
		//	EBUSY (reinitialization)
		//	EPERM (no privileges)
		//	ENOMEM (insufficient memory)
		//	EAGAIN (lacked resources other than memory)
		perror("pthread_mutex_init failed");
		return EXIT_FAILURE;
	}
	mutex_count_initialized = true;

	// initialize the barrier to the number of threads
	if ((errno = pthread_barrier_init(&sync_start_threads, NULL, THREADS))) {
		perror("pthread_barrier_init");
		exit(EXIT_FAILURE);
	}
	sync_start_threads_barrier_initialized = true;

	// create threads
	for (t = 0; t < THREADS; ++t)
		if ((errno = pthread_create(&tid[t], NULL, ThreadAdd, NULL))) {
			perror("pthread_create failed");
			return EXIT_FAILURE;
		}

	// wait for all threads termination
	for (t = 0; t < THREADS; ++t)
		if ((errno = pthread_join(tid[t], NULL))) {
			perror("pthread_join failed");
			return EXIT_FAILURE;
		}

	// resources are released using atexit(3)

	// check the result
	if (count < THREADS * ITERATIONS) {
		fprintf(stderr, "BOOM! count is %d, should be %d\n", count, THREADS * ITERATIONS);
		return EXIT_FAILURE;
	}
	else {
		printf("OK! count is %d\n", count);
		return EXIT_SUCCESS;
	}
}

// EOF
