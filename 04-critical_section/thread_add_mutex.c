// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
// POSIX mutexes:
// pthread_mutex_lock(3), pthread_mutex_unlock(3), pthread_mutex_init(3), pthread_mutex_destroy(3)
// atexit(3)
//
// Modified: 2015-11-11, 2016-11-14, 2017-04-18, 2023-03-29
//
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
}

void *ThreadAdd(void *arg)
{
	int i;
	// register int reg;

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
