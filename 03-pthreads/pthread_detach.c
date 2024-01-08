// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_detach(3) × pthread_join(3), pthread_self(3), pthread_exit(3) × exit(3)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define EXIT_ALL	0	// try 0 and 1
#define JOIN		0	// try 0 and 1

void *thread_function(void *arg)
{
	printf("The created thread's own id:  0x%012lX\n", pthread_self());
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t a_thread;
	int i;

	printf(	"The 1st thread (main) has id: 0x%012lX\n", pthread_self());
	for (i = 0; i < 2; i++) {
		errno = pthread_create(&a_thread, NULL, thread_function, NULL);
		if (errno != 0) {
			perror("pthread_create failed");
			exit(EXIT_FAILURE);
		}
	
#if JOIN
		printf("Join.\n");
		pthread_join(a_thread, NULL);	// no error checking for better readability
#else
		printf("Detach.\n");
		pthread_detach(a_thread);	// no error checking for better readability
#endif
	
		printf("The thread number %d's id was: 0x%012lX\n", i+2, a_thread);
	}

#if EXIT_ALL
	exit(EXIT_SUCCESS);	// exit process
#else
	pthread_exit(NULL);	// exit thread
#endif
}

