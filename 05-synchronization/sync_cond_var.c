// Operating Systems: sample code  (c) B. Nichols, Blaise Barney, Tomáš Hudec
// Synchronization, Condition Variables
// pthread_cond_init(3), pthread_cond_wait(3), pthread_cond_signal(3)

// Note: Error checking is not done

/******************************************************************************
* DESCRIPTION:
*   Example code for using Pthreads condition variables.  The main thread
*   creates three threads.  Two of those threads increment a "count" variable,
*   while the third thread watches the value of "count".  When "count" 
*   reaches a predefined limit, the waiting thread is signaled by one of the
*   incrementing threads. The waiting thread "awakens" and then modifies
*   count. The program continues until the incrementing threads reach
*   TCOUNT. The main program prints the final value of count.
* SOURCE: Adapted from example code in "Pthreads Programming", B. Nichols
*   et al. O'Reilly and Associates. 
* LAST REVISED: 07/16/09  Blaise Barney
* UPDATED: 2010, 2013  Tomáš Hudec
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS	3
#define TCOUNT		10
#define COUNT_THRESHOLD	12

volatile int count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void *inc_count(void *arg)
{
	int i;
	long my_id = *(long *)arg;

	for (i = 0; i < TCOUNT; i++) {
		pthread_mutex_lock(&count_mutex);
		count++;

		/* 
		   Check the value of count and signal waiting thread when
		   condition is reached.  Note that this occurs while mutex
		   is locked. 
		 */
		if (count == COUNT_THRESHOLD) {
			printf("inc_count(): thread %ld, count = %d  Threshold reached. ",
				my_id, count);
			pthread_cond_signal(&count_threshold_cv);
			printf("Just sent signal.\n");
		}
		printf("inc_count(): thread %ld, count = %d, unlocking mutex\n",
			my_id, count);
		pthread_mutex_unlock(&count_mutex);

		// do some work so threads can alternate on mutex lock
		sleep(1);
	}
	pthread_exit(NULL);
}

void *watch_count(void *arg)
{
	long my_id = *(long *)arg;

	printf("Starting watch_count(): thread %ld\n", my_id);

	/*
	   Lock mutex and wait for signal.  Note that the pthread_cond_wait
	   routine will automatically and atomically unlock mutex while it waits. 
	   Also, note that if COUNT_THRESHOLD is reached before this routine is run
	   by the waiting thread, the pthread_cond_wait will be skipped to prevent
	   it from never returning.
	 */
	pthread_mutex_lock(&count_mutex);
	if (count < COUNT_THRESHOLD) {
		printf("watch_count(): thread %ld going into wait...\n", my_id);
		pthread_cond_wait(&count_threshold_cv, &count_mutex);
		printf ("watch_count(): thread %ld Condition signal received.\n",
			my_id);
	}
	// we can do some action here (after the condition is met)
	count += 100;
	printf("watch_count(): thread %ld count now = %d.\n",
		my_id, count);
	pthread_mutex_unlock(&count_mutex);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int i;
	long thread_ids[NUM_THREADS];
	pthread_t threads[NUM_THREADS];

	// initialize mutex and condition variable objects
	pthread_mutex_init(&count_mutex, NULL);
	pthread_cond_init(&count_threshold_cv, NULL);

	// run watching thread
	thread_ids[0] = 1;
	pthread_create(&threads[0], NULL, watch_count, &thread_ids[0]);

	// run counting threads
	for (i = 1; i < NUM_THREADS; i++) {
		thread_ids[i] = i+1;
		pthread_create(&threads[i], NULL, inc_count, &thread_ids[i]);
	}

	// wait for all threads to complete
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("Main(): Waited on %d threads. Final value of count = %d. Done.\n",
		NUM_THREADS, count);

	// clean up and exit
	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&count_threshold_cv);
	return EXIT_SUCCESS;
}
