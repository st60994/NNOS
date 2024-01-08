// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_cancel(3), pthread_cleanup_push(3), pthread_cleanup_pop(3)

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define BUF_SIZE	(1<<6)

pthread_t main_thread;		// id of the main thread

// deallocation of the resource
void release_buffer(void *arg)
{
	free(arg);
	printf("Temporary buffer freed.\n");
}

void *thread_func(void *unused)
{
	char *buf;			// temporary buffer

	if (NULL == (buf = malloc(BUF_SIZE))) {
		perror("malloc");
		return NULL;
	}
	// free upon cancelling/exiting:
	pthread_cleanup_push(release_buffer, buf);

	printf("Temporary buffer allocated.\n");

	printf("Enter a string: ");	// ask for input
	fgets(buf, BUF_SIZE, stdin);	// read input

	pthread_cleanup_pop(true);	// execute cleanup function
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;

	// create another thread which calls / vytvoří další vlákno, které spustí:
	// thread_func(NULL);
	pthread_create(&thread_id, NULL, thread_func, NULL);	// no error checking for lucidity

	sleep(1);
	// after some time, cancel the thread
	printf("\nCancelling the thread.\n");
	if ((errno = pthread_cancel(thread_id)))
		perror("pthread_cancel");

	if ((errno = pthread_join(thread_id, NULL)))
		perror("pthread_join");

	printf("Exiting from main().\n");

	return EXIT_SUCCESS;
}
