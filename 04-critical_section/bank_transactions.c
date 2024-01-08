// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Example code that simulates two sequences of bank transactions in parallel.
// The critical section is the money transfer from one account to the other
// (there may be no money disappearing or suddenly appearing).
//
// Assignment:
// Solve the race condition using a) a POSIX mutex, b) a POSIX (unnamed) semaphore.
// At any time, outside of a critical section, the sum of balances must be AMOUNT.
// Replace the content of the synchronization function with a POSIX barrier.
//
// Zadání:
// Vyřešte problém souběhu použitím a) posixového mutexu, b) posixového (nepojmenovaného) semaforu.
// Kdykoli mimo kritickou sekci musí být součet stavů kont roven částce AMOUNT.
// Nahraďte obsah synchronizační funkce posixovou bariérou.
//
// Modified: 2016-11-14, 2017-04-18, 2023-03-29, 2023-04-06

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <locale.h>

int AMOUNT = 10000;	// default amount

// bank account balances / stav kont
volatile int rich_guy;
volatile int poor_guy;

// DEBUG/info variables:
int trans_per_thread[2];
int amount_per_thread[2];

time_t start_time;

// release all allocated resources, used in atexit(3)
void release_resources(void)
{       
	alarm(0);	// disable info show
}

// synchronize thread start
static void sync_start(void)
{
	while (time(NULL) == start_time) {
		// do nothing except chew CPU slices for up to one second
		// nedělá nic než spotřebovává procesorový čas zjišťováním času
	}
}

#define WAIT_ITERATIONS (100000000/AMOUNT+1)	// simulation of work by iterations
// simulate some work
static void sim_work(void)
{
	int i;
	for (i = 0; i < WAIT_ITERATIONS; ++i);
	// usleep(10);
}

void *bank_transaction(void *arg)
{
	int id = *(int *)arg;	// thread id
	bool done;
	int amount;

	sync_start();		// synchronize threads start / synchronizace startu vláken
	printf("Transaction thread id %d started.\n", *(int *)arg);

	do {
		amount = 1 + rand() % 10;	// choose random amount (1 to 10)
		if (amount > rich_guy)
			amount = rich_guy;
		rich_guy -= amount;
		sim_work();			// simulate some other work
		poor_guy += amount;
		done = rich_guy <= 0 || poor_guy >= AMOUNT;
		trans_per_thread[id]++;		// the number of transactions by particular thread
		amount_per_thread[id] += amount;// transferred amount by particular thread
	} while (!done);

	return NULL;
}

// signal handler: show state
void show_state(int signo)
{
	printf(" rich = %'d, poor = %'d, amount transferred: %'d = %'d + %'d, transactions per thread: %'d + %'d\n",
		rich_guy, poor_guy,
		amount_per_thread[0] + amount_per_thread[1], amount_per_thread[0], amount_per_thread[1],
		trans_per_thread[0], trans_per_thread[1]);
	return;
}

// signal handler: show info
void show_info(int signo)
{
	printf("Press ^\\ (Ctrl+Backslash) to show variables.\n");
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	pthread_t transaction[2];
	int id0 = 0;
	int id1 = 1;

	setlocale(LC_ALL, "");

	// use different initial amount if given on the command-line
	if (argc > 1)
		AMOUNT = strtol(argv[1], NULL, 0);

	// set the initial balance / nastavení počátečního zůstatku
	rich_guy = AMOUNT;
	poor_guy = 0;

	// release resources at program exit
	atexit(release_resources);

	start_time = time(NULL);
	srand(start_time + getpid());

	printf("%s:\nSimulation of concurrent bank transactions.\n", argv[0]);
	// id

	printf("Rich guy’s account balance: %10d\n"
	       "Poor guy’s account balance: %10d\n"
	       "Starting two parallel bank transfers...\n", rich_guy, poor_guy);

	// show variables upon pressing ^\ (CTRL+Backslash)
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
	sa.sa_handler = show_state;
	sigaction(SIGQUIT, &sa, NULL);
	sa.sa_handler = show_info;
	sigaction(SIGALRM, &sa, NULL);
	alarm(2);	// if not finished within 2 seconds, show info

	if ((errno = pthread_create(&transaction[0], NULL, bank_transaction, &id0))) {
		perror("ERROR creating thread 1");
		return EXIT_FAILURE;
	}
	if ((errno = pthread_create(&transaction[1], NULL, bank_transaction, &id1))) {
		perror("ERROR creating thread 2");
		return EXIT_FAILURE;
	}

	printf("Waiting for transactions to complete...\n");
	(void) pthread_join(transaction[0], NULL);
	(void) pthread_join(transaction[1], NULL);
	// return value is not checked, possible errors that should not happen:
	//	EINVAL (not a valid thread ID || another thread is already waiting to join with this thread)
	//	EDEADLK (two threads tried to join with each other)
	//	ESRCH (no thread with the given ID found)

	alarm(0);	// disable info show

	printf("Done.\n"
	       "Rich guy’s account balance: %10d\n"
	       "Poor guy’s account balance: %10d\n"
	       "Sum of account balances:    %10d (%s)\n"
	       "Transferred amount:         %10d (%s) = %d + %d\n"
	       "Transactions per thread:    %10d = %d + %d\n",
	       rich_guy, poor_guy, rich_guy + poor_guy,
	       AMOUNT == rich_guy + poor_guy ? "OK" : "BAD",
	       amount_per_thread[0] + amount_per_thread[1],
	       AMOUNT == amount_per_thread[0] + amount_per_thread[1] ? "OK" : "BAD",
	       amount_per_thread[0],
	       amount_per_thread[1],
	       trans_per_thread[0] + trans_per_thread[1],
	       trans_per_thread[0],
	       trans_per_thread[1]);

	// resources are released using atexit(3)

	if (0 != rich_guy || AMOUNT != poor_guy || AMOUNT != amount_per_thread[0] + amount_per_thread[1]) {
		fprintf(stderr, "\nBAD TRANSACTIONS DETECTED!\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// EOF
