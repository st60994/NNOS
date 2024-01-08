// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization, Barriers
// pthread_barrier_init(3), pthread_barrier_wait(3), pthread_barrier_destroy(3)

// Modified: 2023-04-06

// A barrier is a tool that allows the simultaneous execution of threads from the given point.

// enable barriers
// FIXME

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

// a barrier declaration
// FIXME

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	// FIXME
}

void *thread_func(void *arg)
{
	int id = *(int *)arg;
	struct timeval tv;
	struct tm localtime;			// for strftime
	char timestr[MAX_DATE_STR_LEN];

	printf("The thread %d is working.\n", id);

	// sleep for some random amount of time (simulate work)
	usleep( 1000 * (1 + (int) (6000.0 * (rand() / (RAND_MAX + 1.0)))) );

	gettimeofday(&tv, NULL);		// get current time, microsecond precision

	localtime_r(&tv.tv_sec, &localtime);	// transform time to broken down format (for strftime)
	strftime(timestr, sizeof(timestr), TIME_FMT, &localtime);
	printf("The thread %d is waiting for the other threads: %s.%03ld\n", id, timestr, tv.tv_usec/1000);

	// wait at the barrier
	// FIXME

	gettimeofday(&tv, NULL);		// get current time, microsecond precision

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

	// initialize the barrier
	// FIXME

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
