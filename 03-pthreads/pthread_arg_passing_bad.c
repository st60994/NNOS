// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// argument passing: incorrect way
// předávání parametru: nesprávný způsob
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// usleep

#define THREADS 10

void *print_arg(void *arg)
{
	// usleep(100);		// try to uncomment / zkuste odkomentovat
	printf("Thread arg: %d\n", *(int *)arg);
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_ids[THREADS];
	int i;

	for (i = 0; i < THREADS; i++) {
		// create a thread with the argument as the reference to the i variable
		// vytvoření vlákna s argumentem ukazatele na proměnnou i
		pthread_create(&thread_ids[i], NULL, print_arg, &i);
		// no error checking for lucidity
	}

	for (i = 0; i < THREADS; i++)
		pthread_join(thread_ids[i], NULL);

	return EXIT_SUCCESS;
}
