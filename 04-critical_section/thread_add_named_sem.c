// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
// POSIX named semaphores:
// sem_open(3), sem_wait(3), sem_post(3), sem_unlink(3), sem_close(3)
// atexit(3)
//
// Modified: 2015-11-11, 2016-11-14, 2017-04-18, 2023-03-29
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>		// POSIX semaphores
#include <fcntl.h>		// open flags: O_CREAT
#include <sys/stat.h>		// permissions: S_IRUSR, S_IWUSR

#define SEM_NAME	"/thread_add-st60094"	// replace login with your real login name

#define THREADS 2		// the number of threads
#define ITERATIONS 1000000	// the number of operations per thread

volatile int count = 0;		// shared variable

// declare a semaphore for critical section with the ‘count’ variable
sem_t *psem_cs_count = SEM_FAILED;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (SEM_FAILED != psem_cs_count) {
		// close the semaphore
		if (sem_close(psem_cs_count))
			perror("sem_close");
		else
			psem_cs_count = SEM_FAILED;
		sem_unlink(SEM_NAME);
	}
}

void *ThreadAdd(void *arg)
{
	int i;
	// register int reg;

	for (i = 0; i < ITERATIONS; ++i) {
		// ENTRY SECTION
		sem_wait(psem_cs_count);
		// return value is not checked, possible errors that should not happen:
		//	EINVAL (not a valid semaphore)		– semaphore is valid and initialized
		//	EINTR (interrupted by a signal handler)	– signal handlers are not used,
		//		the SA_RESTART flag ensures that EINTR never happens (since Linux 2.6.22)
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
		sem_post(psem_cs_count);
		// return value is not checked, possible errors that should not happen:
		//	EINVAL (not a valid semaphore)	– semaphore is valid and initialized
		//	EOVERFLOW			– post is always called after wait
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid[THREADS];
	int t;

	atexit(release_resources);	// release resources at program exit

	// initialize the semaphore counter to 1
	// parameters:
	//   semaphore name,
	//   flags: O_CREAT = create a new semaphore if this name does not exist,
	//   mode: permissions,
	//   counter value
	if (SEM_FAILED == (psem_cs_count = sem_open(SEM_NAME, O_CREAT, S_IRUSR|S_IWUSR, 1))) {
		perror("sem_open");
		return EXIT_FAILURE;
	}

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
