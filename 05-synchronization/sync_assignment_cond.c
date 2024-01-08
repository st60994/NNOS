// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization: Semaphores / Condition Variables / Message Queues

// Modified: 2016-11-21, 2021-11-23

// Assignment:
//
// The program has to print the numbers in the increasing order (1, 2)
// independently on the given sleep value. The order has to be preserved
// even if another sleep is inserted at arbitrary position in the program.
// Synchronize using
//	a) POSIX condition variable,
//	b) POSIX semaphore,
//	c) System V semaphore,
//	d) POSIX message queue,
//	e) System V message queue.

// Zadání:
//
// Program musí vypsat čísla v pořadí 1, 2 nezávisle na zadané hodnotě
// čekáni sleep. Pořadí musí zůstat zachováno i v případě vložení dalšího
// sleep na jakékoliv místo v programu.
// Synchronizujte použitím
//	a) posixové podmínkové proměnné,
//	b) posixového semaforu,
//	c) semaforu System V,
//	d) posixové fronty zpráv,
//	e) fronty zpráv System V.

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>

int sleep_time = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;


// print by the second thread, called from thread_f1()
void f1(void)
{
	sleep(sleep_time);	// simulation of some work

	pthread_mutex_lock(&count_mutex);
	printf("1\n");		// MUST be printed FIRST
	pthread_cond_signal(&count_threshold_cv);
	pthread_mutex_unlock(&count_mutex);
	// post
	return;
}

// print by the first thread, called from main()
void f2(void)
{
	// wait
	pthread_mutex_lock(&count_mutex);
	pthread_cond_wait(&count_threshold_cv, &count_mutex);
	printf("2\n");		// MUST be printed AFTER printing 1
	pthread_mutex_unlock(&count_mutex);
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
	pthread_t thread;
	int rc;
	pthread_mutex_init(&count_mutex, NULL);
	pthread_cond_init (&count_threshold_cv, NULL);


	if (argc > 1) {
		sleep_time = atoi(argv[1]);
	}
	// printf("Waiting for sleep_time = %d seconds.\n", sleep_time);


	rc = pthread_create(&thread, NULL, thread_f1, NULL);
	if (rc) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	f2();

	(void) pthread_join(thread, NULL);
	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&count_threshold_cv);


	return 0;
}

// EOF
