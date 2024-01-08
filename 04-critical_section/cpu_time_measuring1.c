// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, CPU time
// getrusage(2)
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

#include <sys/time.h>		// CPU time measuring
#include <sys/resource.h>	// CPU time measuring

#define THREADS		2	// the number of threads used
#define ITERATIONS	1<<20	// the number of loops

pthread_barrier_t a_barrier;	// for simultaneous start

volatile long int count;	// a shared variable

volatile int locked = 0;	// a control variable

struct rusage CPU_time1, CPU_time2;	// counting the CPU clocks
double CPU_time_user, CPU_time_system;	// time spent on the CPU

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
			getrusage(RUSAGE_SELF, &CPU_time1);	// initialize the CPU time
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
			fprintf(stderr, "ERROR creating thread 1\n");
			return EXIT_FAILURE;
		}
	}

	// wait for threads termination
	for (i = 0; i < THREADS; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "ERROR joining thread 1\n");
			return EXIT_FAILURE;
		}
	}

	// calculate the CPU time used by threads
	getrusage(RUSAGE_SELF, &CPU_time2);

	CPU_time_user =
		(double) (CPU_time2.ru_utime.tv_sec  - CPU_time1.ru_utime.tv_sec) +
		(double) (CPU_time2.ru_utime.tv_usec - CPU_time1.ru_utime.tv_usec) / 1000000.0;
	CPU_time_system =
		(double) (CPU_time2.ru_stime.tv_sec  - CPU_time1.ru_stime.tv_sec) +
		(double) (CPU_time2.ru_stime.tv_usec - CPU_time1.ru_stime.tv_usec) / 1000000.0;

	// release the system resources allocated by the barrier
	if (pthread_barrier_destroy(&a_barrier)) {
		perror("pthread_barrier_destroy");
		return EXIT_FAILURE;
	}

	// print the time used
	printf("The total time spent on the CPU(s): %.0lf ms\n", (CPU_time_user + CPU_time_system) * 1000);

	return EXIT_SUCCESS;
}
