// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Modified: 2015-12-10, 2018-11-05, 2023-03-29

#include <stdio.h>
#include <stdlib.h>			// srand(3), rand(3)
#include <sys/types.h>
#include <unistd.h>			// getpid()
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>			// bool, true, false

#define INITIAL_AMOUNT	(1<<22)		// initial balance
#define THREADS		(1<<2)		// the number of concurrent threads
#define MAX_WITHDRAW	(1<<6)		// maximum amount per transaction

volatile int balance = INITIAL_AMOUNT;	// shared variable, initial balance

int withdrawn[THREADS];			// the amount withdrawn by each thread

int verbose = 1;			// verbosity

// critical section variables


// synchronization variables


// synchronize start of all threads
// synchronizace startu vláken
static void sync_threads(void)
{
	
}

// withdraw given amount, returns true if the transaction was successful, false otherwise
// výběr dané částky, vrací true, pokud byla transakce úspěšná, jinak false
__attribute__ ((always_inline))	// ask for optimization
static inline			// not used unless asked for optimization
bool withdraw(int amount) {
	// check if the transaction can be done
	if (balance < amount)	// if not enough: reject withdrawal
		return false;
	balance -= amount;	// do withdrawal
	return true;
}

void *do_withdrawals(void *arg)
{
	int id = *(int *)arg;
	int i;
	int amount;
	bool finished;

	sync_threads();		// synchronize threads start / synchronizace startu vláken

	// each thread makes at most (INITIAL_AMOUNT / THREADS) withdrawals
	for (i = 0, finished = false; i < INITIAL_AMOUNT / THREADS && !finished; ++i) {

		// random amount: 1 to MAX_WITHDRAW
		amount = 1 + (int) (MAX_WITHDRAW * 1.0 * (rand() / (RAND_MAX + 1.0)));

		if (withdraw(amount))	// do the transaction
			withdrawn[id] += amount;	// success, sum up total
		else	// not enough resources left
			if (verbose > 1)
				fprintf(stderr, "Transaction rejected: %d, %d\n", balance, -amount);
		// set finished flag if no resources left
		finished = balance <= 0;
	}

	if (verbose > 1)
		printf("Thread %2d: transactions performed: %9d\n", id, i);

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tids[THREADS];
	int t[THREADS];
	int i;
	int total_withdrawn = 0;

	// id

	// initialization


	srand(getpid() * time(NULL));	// RNG init

	// report initial state
	printf("%-20s %9d\n", "The initial balance:", balance);

	// create threads
	for (i = 0; i < THREADS; ++i) {
		t[i] = i;
		if ((errno = pthread_create(&tids[i], NULL, do_withdrawals, &t[i]))) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	if (verbose)
		printf("Threads started: %d\n", i);

	// wait for the threads termination
	for (i = 0; i < THREADS; ++i) {
		if ((errno = pthread_join(tids[i], NULL))) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}
		// sum up the total withdrawn amount by each thread
		total_withdrawn += withdrawn[i];
		if (verbose)
			printf("%-20s %9d\n", "Thread withdrawal:", withdrawn[i]);
	}

	// report the total amount withdrawn and the new state
	printf("%-20s %9d\n", "The new balance:", balance);
	printf("%-20s %9d\n", "Total withdrawn:", total_withdrawn);

	// check the result and report
	if (balance + total_withdrawn != INITIAL_AMOUNT)
		fprintf(stderr, "LOST TRANSACTIONS DETECTED!\n"
				"initial − new != total withdrawal (%d != %d)\n",
				INITIAL_AMOUNT - balance, total_withdrawn);

	return EXIT_SUCCESS;
}

