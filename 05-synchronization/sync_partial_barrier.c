// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization, Barriers
// pthread_barrier_init(3), pthread_barrier_wait(3), pthread_barrier_destroy(3)

// Modified: 2023-04-06

// A barrier is a tool that allows the simultaneous execution of threads from the given point.

// PARTIALLY SOLVED
// Replace all the “FIXME” words with the correct word/number
// and remove comments around completed commands.

#if !defined _XOPEN_SOURCE || _XOPEN_SOURCE < 600
#	define _XOPEN_SOURCE 600	// enable barriers
#endif

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <stdlib.h>

#define THREADS 10			// the total number of threads

#define	TIME_FMT		"%F %T"	// string format for time
#define MAX_DATE_STR_LEN	20	// max string length of date + time

#define MAX_// synchronization barrier for simultaneous run of all threads
pthread_barrier_t sync_threads;
bool sync_threads_barrier_initialized = false;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (sync_threads_barrier_initialized) {
		// release the system resources allocated by the barrier
		/*
		if ((errno = pthread_barrier_FIXME(FIXME)))
			perror("pthread_barrier_FIXME");
		else
		*/
			sync_threads_barrier_initialized = false;
	}
}

void *thread_func(void *arg)
{
	int id = *(int *)arg;
	struct timeval tv;
	struct tm localtime;			// for strftime
	char timestr[MAX_DATE_STR_LEN];
	int rc = 0;

	printf("The thread %d is working.\n", id);

	// sleep for some random amount of time (simulate work)
	usleep( 1000 * (1 + (int) (6000.0 * (rand() / (RAND_MAX + 1.0)))) );

	gettimeofday(&tv, NULL);		// get current time, microsecond precision

	localtime_r(&tv.tv_sec, &localtime);	// transform time to broken down format (for strftime)
	strftime(timestr, sizeof(timestr), TIME_FMT, &localtime);
	printf("The thread %d is waiting for the other threads: %s.%03ld\n", id, timestr, tv.tv_usec/1000);

	// wait at the barrier
	/*
	rc = pthread_barrier_FIXME(FIXME);
	*/

	gettimeofday(&tv, NULL);		// get current time, microsecond precision

	switch (errno = rc) {	// check the return code
		case PTHREAD_BARRIER_SERIAL_THREAD:
			// this will be executed only once (by only one thread)
			printf("All threads are resumed: %ld.%03ld\n", tv.tv_sec, tv.tv_usec/1000);
			// we can destroy the barrier here
		case 0:
			// this will be executed by all threads
			break;
		default:// error
			perror("pthread_barrier_FIXME");
			exit(EXIT_FAILURE);
	}

	localtime_r(&tv.tv_sec, &localtime);	// transform time to broken down format (for strftime)
	strftime(timestr, sizeof(timestr), TIME_FMT, &localtime);
	printf("The thread %d is resumed: %s.%03ld\n", id, timestr, tv.tv_usec/1000);

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t threads[THREADS];
	int arguments[THREADS];
	int t;

	// initialize random number genereator
	srand(time(NULL) * getpid());

	atexit(release_resources);	// release resources at program exit

	// initialize the barrier to the number of threads
	/*
	if ((errno = pthread_barrier_FIXME(&sync_threads, NULL, FIXME))) {
		perror("pthread_barrier_FIXME");
		exit(EXIT_FAILURE);
	}
	*/
	sync_threads_barrier_initialized = true;

	// create threads
	for (t = 0; t < THREADS; ++t) {
		arguments[t] = t;
		if ((errno = pthread_create(&threads[t], NULL, thread_func, &arguments[t]))) {
			perror("pthread_create failed");
			exit(EXIT_FAILURE);
		}
	}

	// wait for all threads to terminate
	for (t = 0; t < THREADS; ++t) {
		(void) pthread_join(threads[t], NULL);
	}

	// resources are released using atexit(3)

	return EXIT_SUCCESS;
}

// EOF
