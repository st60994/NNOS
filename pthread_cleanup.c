// Operating Systems: sample code  (c) Tomáš Hudec
// Signals, Threads
// pthread_cancel(3), pthread_cleanup_push(3), pthread_cleanup_pop(3)
//
// Remove errors (compiler warnings), correctly deallocate resources (only allocated ones).
// Use pthread_cleanup_push(3), pthread_cleanup_pop(3) for freeing a temporary buffer.
//
// Modified: 2023-11-09

// enable sem_timedwait
#if _XOPEN_SOURCE < 600
#	define _XOPEN_SOURCE 600
#endif

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>

#define TIMEOUT		5	// timeout for entering data
bool timeout_gone = false;
sem_t thread_finished;
bool thread_finished_initialized = false;
bool fgets_exited_with_error = false;
#define BUF_SIZE	(1<<5)
char *name = NULL;		// buffer for name storage

// free a buffer (passed as the address of the pointer to buffer), store NULL to it and print optional message
void release_buffer(char **buf, char *msg)
{
	if (buf && *buf != NULL) {		//validate buffer was allocated and contains something to require freeing
		free(*buf);
		*buf = NULL;
		if (msg)
			printf("%s\n", msg);
	}
}

// deallocation of the resources
void release_resources(void)
{
	// semaphore release
	if (thread_finished_initialized) {
		if (sem_destroy(&thread_finished))
			perror("sem_destroy");
		else
			thread_finished_initialized = false;
	}
	// name buffer release
	release_buffer(&name, "Name buffer freed.");
}

void release_buffer_tmp(void *arg)
{
	release_buffer(arg, "Temporary buffer freed.");
}

void *thread_func(void *unused)
{
	char *buf = NULL;		// temporary buffer for reading a string

	if (NULL == (buf = malloc(BUF_SIZE))) {	// allocate a temporary buffer
		perror("malloc");
		return NULL;
	}
	printf("Temporary buffer allocated.\n");
	pthread_cleanup_push(release_buffer_tmp, &buf);

	printf("Enter your name (timeout %d s): ", TIMEOUT);	// ask for input
	if (fgets(buf, BUF_SIZE, stdin) == NULL) {			// read input and save it to buffer
		fgets_exited_with_error = true;		
	}else{
		name = strdup(buf); 		// allocate memory for entered name
		printf("Name buffer allocated.\n");
	}
	pthread_cleanup_pop(1);	// execute cleanup handler to free the temporary buffer
	sem_post(&thread_finished);	// notify about finishing
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;
	struct timeval now;
	struct timespec timeout;

	// remove all allocated resources upon exit
	atexit(release_resources);

	// initialize a semaphore, thread_shared, counter to 0
	if (sem_init(&thread_finished, 0, 0)) {
		perror("sem_init");
		return EXIT_FAILURE;
	}
	thread_finished_initialized = true;

	// create another thread
	pthread_create(&thread_id, NULL, thread_func, NULL);	// no error checking for lucidity

	// do some other job

	// impose a timeout for the thread:
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + TIMEOUT;	// wait at most TIMEOUT seconds
	timeout.tv_nsec = now.tv_usec * 1000;
	// timed wait for signal:
	if (sem_timedwait(&thread_finished, &timeout)) {
		timeout_gone = ETIMEDOUT == errno;
		if (!timeout_gone)
			perror("sem_timedwait");
	}
	if (fgets_exited_with_error) {
		perror("fgets");		// prints what error occured in fgets
	}
	// check the timeout
	else if (timeout_gone) {
		fprintf(stderr, "timeout gone\n");
		if ((errno = pthread_cancel(thread_id)))	// cancel thread execution
			perror("pthread_cancel");
		else
			fprintf(stderr, "Thread execution was cancelled.\n");
	}
	else
		printf("Entered name: %s\n", name);

	if ((errno = pthread_join(thread_id, NULL)))
		perror("pthread_join");

	printf("Exiting from main().\n");

	return EXIT_SUCCESS;
}
