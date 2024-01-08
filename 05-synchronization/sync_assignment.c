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

// print by the second thread, called from thread_f1()
void f1(void)
{
	sleep(sleep_time);	// simulation of some work

	printf("1\n");		// MUST be printed FIRST

	return;
}

// print by the first thread, called from main()
void f2(void)
{
	printf("2\n");		// MUST be printed AFTER printing 1

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

	return 0;
}

// EOF
