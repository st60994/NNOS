// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, CPU time
// clock(3)
//
// Modified: 2013-12-11, 2018-11-08
//
// we need to define _XOPEN_SOURCE >= 600 for portability (to access barriers)
// alternatively we can define _POSIX_C_SOURCE:
// #define _POSIX_C_SOURCE 200112L	// this also enables barriers
#define _XOPEN_SOURCE	600		// enable barriers

#define GIVE_UP_CPU	1		// set to 1 for sched_yield() in busy wait

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#if GIVE_UP_CPU	> 0

// POSIX systems on which sched_yield() is available
// define _POSIX_PRIORITY_SCHEDULING in <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>		// sched_yield(2)
#endif

#endif

#include <time.h>		// CPU time measuring

#define THREADS		5	// the number of threads used
#define ITERATIONS	1<<22	// the number of loops

pthread_barrier_t a_barrier;	// for simultaneous start

volatile long int count;	// a shared variable

volatile int locked = 0;	// a control variable

clock_t CPU_clocks;		// counting the CPU clocks
double CPU_time;		// time spent on the CPU

// wait on the barrier
static void wait_thread(void)
{
	int rc;

	// wait on the barrier
	rc = pthread_barrier_wait(&a_barrier);

	switch (rc) {	// check the return code
		case PTHREAD_BARRIER_SERIAL_THREAD:
			// this will be executed only once (by only one thread)
			// we can destroy a barrier here
			CPU_clocks = clock();	// initialize the CPU clocks
		case 0:
			// this will be executed by all threads
			break;
		default:
			// error
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
	}

}

void *thread_func(void *arg)
{
	long int i;

	wait_thread();		// synchronize threads start

	for (i = 0; i < ITERATIONS; ++i) {
		while (locked) {	// busy wait
#if GIVE_UP_CPU	> 0
			// give up the CPU during the wait
#ifdef _POSIX_PRIORITY_SCHEDULING
			sched_yield();
#else
			usleep(1);
#endif
#endif
		}
		locked = 1;
		count++;
		locked = 0;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	pthread_t tid[THREADS];

	// initialize the barrier to the number of threads
	if (pthread_barrier_init(&a_barrier, NULL, THREADS)) {
		perror("pthread_barrier_init");
		return EXIT_FAILURE;
	}

#if GIVE_UP_CPU	> 0
#ifdef _POSIX_PRIORITY_SCHEDULING
	fprintf(stderr, "sched_yield() supported and used\n");
#else
	fprintf(stderr, "sched_yield() not supported, 'usleep(1);' used instead\n");
#endif
#endif

	// create threads
	for (i = 0; i < THREADS; ++i) {
		if (pthread_create(&tid[i], NULL, thread_func, NULL)) {
			fprintf(stderr, "ERROR creating thread %d\n", i);
			return EXIT_FAILURE;
		}
	}

	// wait for threads termination
	for (i = 0; i < THREADS; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "ERROR joining thread %d\n", i);
			return EXIT_FAILURE;
		}
	}

	// calculate the CPU time used by threads
	CPU_time = (double) (clock() - CPU_clocks) / CLOCKS_PER_SEC;

	// release the system resources allocated by the barrier
	if (pthread_barrier_destroy(&a_barrier)) {
		perror("pthread_barrier_destroy");
		return EXIT_FAILURE;
	}

	// print the time used
	printf("The total time spent on the CPU(s): %.0lf ms\n", CPU_time * 1000);

	return EXIT_SUCCESS;
}
