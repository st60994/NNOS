// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, CPU time, real time
// getrusage(2), clock_gettime(3)
//
// Modified: 2015-12-17, 2017-11-30

#if !defined _XOPEN_SOURCE || _XOPEN_SOURCE < 600
#	define _XOPEN_SOURCE 600	// enable barriers
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define GIVE_UP_CPU true		// give up the CPU during a busy wait loop?

#if GIVE_UP_CPU
	// POSIX systems on which sched_yield() is available
	// define _POSIX_PRIORITY_SCHEDULING in <unistd.h>
#	ifdef _POSIX_PRIORITY_SCHEDULING
#		include <sched.h>	// sched_yield(2)
#	endif
#endif

#include <time.h>			// real time measuring
#include <sys/time.h>			// CPU time measuring
#include <sys/resource.h>		// CPU time measuring

#define THREADS		4		// the number of threads used
#define ITERATIONS	(1<<26)		// the number of loops

pthread_barrier_t a_barrier;		// for simultaneous start
bool barrier_initialized = false;

volatile long int count;		// a shared variable

volatile int locked = 0;		// a control variable

struct timespec real_time1, real_time2;	// counting the real time
struct rusage CPU_time1, CPU_time2;	// counting the CPU clocks
double real_time;			// time spent executing the process
double CPU_time_user, CPU_time_system;	// time spent on the CPU

// wait at the barrier
static void sync_threads(void)
{
	switch (pthread_barrier_wait(&a_barrier)) {		// wait at the barrier
		case PTHREAD_BARRIER_SERIAL_THREAD:		// OK, once
			clock_gettime(CLOCK_MONOTONIC, &real_time1);	// real time init
			getrusage(RUSAGE_SELF, &CPU_time1);	// initialize the CPU time
		case 0:	break;		// OK, all threads
		default:		// error
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
	}
}

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (barrier_initialized) {
		// destroy the barrier
		if (pthread_barrier_destroy(&a_barrier))
			perror("pthread_barrier_destroy");
		else
			barrier_initialized = false;
	}
}

void *thread_func(void *arg)
{
	long int i;

	sync_threads();			// synchronize threads start

	for (i = 0; i < ITERATIONS; ++i) {
		while (locked) {	// busy wait
#if GIVE_UP_CPU
			// give up the CPU during the busy wait
#	ifdef _POSIX_PRIORITY_SCHEDULING
			sched_yield();
#	else
			usleep(1);
#	endif
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

	// release used resources automatically upon exit
	atexit(release_resources);

	// initialize the barrier to the number of threads
	if (pthread_barrier_init(&a_barrier, NULL, THREADS)) {
		perror("pthread_barrier_init");
		return EXIT_FAILURE;
	}
	barrier_initialized = true;

#ifdef _POSIX_PRIORITY_SCHEDULING
	fprintf(stderr, "sched_yield() supported %s used\n", GIVE_UP_CPU ? "and" : "but NOT");
#else
	fprintf(stderr, "sched_yield() not supported%s\n", GIVE_UP_CPU ? ", 'usleep(1);' used instead" : " and not used");
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

	// calculate the CPU and real time used by threads
	getrusage(RUSAGE_SELF, &CPU_time2);
	clock_gettime(CLOCK_MONOTONIC, &real_time2);

	// substract init time, merge the values (seconds and microseconds)
	real_time =
		(double) (real_time2.tv_sec  - real_time1.tv_sec) +
		(double) (real_time2.tv_nsec - real_time1.tv_nsec) / 1000000000.0;
	CPU_time_user =
		(double) (CPU_time2.ru_utime.tv_sec  - CPU_time1.ru_utime.tv_sec) +
		(double) (CPU_time2.ru_utime.tv_usec - CPU_time1.ru_utime.tv_usec) / 1000000.0;
	CPU_time_system =
		(double) (CPU_time2.ru_stime.tv_sec  - CPU_time1.ru_stime.tv_sec) +
		(double) (CPU_time2.ru_stime.tv_usec - CPU_time1.ru_stime.tv_usec) / 1000000.0;

	// print the time used
	printf("The time spent on the CPU(s) in milliseconds (real user system): "
		"%.0lf %.0lf %.0lf\n",
		real_time * 1000,
		CPU_time_user * 1000,
		CPU_time_system * 1000);

	return EXIT_SUCCESS;
}
