// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization, Semaphores
// sem_init(3), sem_wait(3), sem_post(3), sem_destroy(3)

// Modified: 2014-12-01, 2023-04-04

// Assignment:
//
// The program must print the numbers in the increasing order (1, 2)
// independently on the given sleep value.
// Synchronize using the semaphores.
//
// PARTIALLY solved, just replace the word FIXME with correct text.
//

// Zadání:
//
// Program musí vypsat čísla v pořadí 1, 2 nezávisle na zadané hodnotě
// čekáni sleep. Synchronizujte použitím semaforu.
//
// ČÁSTEČNĚ vyřešeno, nahraďte výrazy FIXME správným textem.
//

#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>

#include <semaphore.h>		// POSIX semaphores

sem_t sync_print;		// a semaphore for synchronization of the print statements
//#define SEM_SYNC_INIT_VALUE	FIXME
bool sync_print_initialized = false;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	if (sync_print_initialized) {
		// release the system resources allocated by the semaphore
		if (sem_destroy(&sync_print))
			perror("sem_destroy");
		else
			sync_print_initialized = false;
	}
}

int sleep_time = 0;

// print by the second thread, called from thread_f1()
void f1(void)
{
	sleep(sleep_time);	// simulation of some work

	// MUST be printed FIRST
	printf("1\n");
//	sem_FIXME(&sync_print);

	return;
}

// print by the first thread, called from main()
void f2(void)
{
//	sem_FIXME(&sync_print);
	// MUST be printed AFTER printing 1
	printf("2\n");

	return;
}

// 2nd thread
void *thread_f1(void *arg)
{
	f1();
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;

	atexit(release_resources);	// release resources at program exit

	if (argc > 1) {
		sleep_time = atoi(argv[1]);
	}
	// printf("Waiting for sleep_time = %d seconds.\n", sleep_time);

	// semaphore initializtion
	// parameters:
	//	pointer to the semaphore,
	//	share type (0 = threads only, 1 = processes),
	//	counter initial value
/*
	if (sem_FIXME(&sync_print, 0, SEM_SYNC_INIT_VALUE)) {
		perror("sem_FIXME");
		exit(EXIT_FAILURE);
	}
*/
	sync_print_initialized = true;

	if ((errno = pthread_create(&thread_id, NULL, thread_f1, NULL))) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	f2();

	(void) pthread_join(thread_id, NULL);

	// resources are released using atexit(3)

	return EXIT_SUCCESS;
}

// EOF
