// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Example code that simulates several sequences of bank deposit operations
// in parallel.
//
// Modified: 2014-11-13

#define _XOPEN_SOURCE 600	// for barriers

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		// getopt(3)
#include <errno.h>
#include <string.h>		// strerror(3)

#define MAX_THREADS			100

int sync_start = 0;		// threads parallel start?
pthread_barrier_t bariera;

int verbose = 1;		// 0 = quiet mode

// default values
#define ACCOUNT_START			1000000
#define BANK_TRANSACTION_AMOUNT		100
#define BANK_TRANSACTION_FEE		1
#define BANK_TRANSACTIONS_PER_THREAD	10000
#define THREAD_COUNT			3

// account ballance in the beginning
volatile long long int account_balance = ACCOUNT_START;

// the amount of money to be deposited
long long int bank_transaction_amount = BANK_TRANSACTION_AMOUNT;

// transaction fee per deposit
long long int bank_transaction_fee = BANK_TRANSACTION_FEE;

// the number of transactions in one thread
unsigned int bank_transactions_per_thread = BANK_TRANSACTIONS_PER_THREAD;

// the number of thread
unsigned int thread_count = THREAD_COUNT;


// per thread structure of thread IDs and arguments
struct thread_info_t {
	pthread_t id;
	int arg;
} thread_info[MAX_THREADS];

// prototypes
void eval_args(int argc, char *argv[]);

// variable declaration / function definitions
// for the critical section access control



// do bank operation, save record and return new account balance
// the value parameter is the value to be deposited
long long int deposit_money(long long int value)
{
	// save record about the transaction (not included)

	if (verbose > 2)
		printf("Account balance before: %lld, credit: %lld, fee: %lld\n",
			account_balance, value, bank_transaction_fee);

	return account_balance + value - bank_transaction_fee;	// new balance
}


// thread function for transactions
// the arg parameter is a pointer to the thread id
void *bank_deposit(void *arg)
{
	int i;
	int id = *(int *) arg;

	if (sync_start)
		// if requested, start all transactions in parallel
		switch (pthread_barrier_wait(&bariera)) {
		  case PTHREAD_BARRIER_SERIAL_THREAD:
			printf("Threads start transactions.\n");
		  case 0:
			break;
		  default:
			perror("pthread_barrier_wait");
			exit(3);
		}
	
	if (verbose > 1)
		printf("The thread %d starts transactions.\n", id);

	// do transactions in a loop
	for (i = 0; i < bank_transactions_per_thread; i++) {

		// save the new account balance returned from the deposit_money()
		account_balance = deposit_money(bank_transaction_amount);

	}

	if (verbose > 1)
		printf("The thread %d finished transactions.\n", id);

	return NULL;
}

// main thread
int main(int argc, char *argv[])
{
	int i;
	long long int expected;		// expected account ballance

	// initialization of variables used for the critical section access control


	// evaluate command-line arguments
	eval_args(argc, argv);

	// calculate the expected account balance
	expected =
	    account_balance +
	    thread_count * bank_transactions_per_thread *
		(bank_transaction_amount - bank_transaction_fee);

	// initialize a barrier
	if (sync_start)
		if (pthread_barrier_init(&bariera, NULL, thread_count)) {
			perror("pthread_barrier_init");
			return 3;
		}

	if (verbose)
		printf("Account balance before: %10lld\n", account_balance);

	for (i = 0; i < thread_count; i++) {
		thread_info[i].arg = i;
		// do transactions in parallel using threads
		if (pthread_create(&thread_info[i].id, NULL, bank_deposit,
				   &thread_info[i].arg)) {
			// error creating a thread
			perror("pthread_create");
			return 3;
		}
	}

	// wait for thread terminations
	for (i = 0; i < thread_count; i++) {
		pthread_join(thread_info[i].id, NULL);
	}

	if (verbose)
		printf("Account balance after:  %10lld\n", account_balance);

	// compare the result with the expected value
	if (account_balance != expected) {
		fprintf(stderr,
			"ERROR IN TRANSACTIONS! Ballance: %lld, expected: %lld\n",
			account_balance, expected);
		return 1;
	}

	return 0;
}

// help
void usage(FILE * stream, char *self)
{
	fprintf(stream,
		"Usage:\n"
		"  %s -h\n"
		"  %s [-q|-v] [-w] [-c count] [-t trans] [-a amount] [-f fee] [-s state]\n"
		"Purpose:\n"
		"  The simulation of bank operations made in parallel on the account.\n"
		"Options:\n"
		"  -h		help\n"
		"  -c count	the number of threads (%u, max. %d)\n"
		"  -t trans	the number of transactions per one thread (%u)\n"
		"  -a amount	the amount to be deposited by every transaction (%lld)\n"
		"  -f fee	the bank fee per transaction (%lld)\n"
		"  -s state	initial account ballance (%lld)\n"
		"  -w		wait: parallel start of threads\n"
		"  -q		be more quiet, no message about account ballance\n"
		"  -v		be more verbose\n",
		self, self,
		thread_count, MAX_THREADS,
		bank_transactions_per_thread,
		bank_transaction_amount,
		bank_transaction_fee,
		account_balance);
}

// evaluate the command-line switches
void eval_args(int argc, char *argv[])
{
	int opt;		// option

	opterr = 0;		// don't print error message on unknown switches, we take care of it
	// switches processing
	while ((opt = getopt(argc, argv, "hwqvc:t:a:f:s:")) != -1) {
		switch (opt) {
		// -c count = number of threads
		case 'c':
			thread_count = atoi(optarg);
			if (thread_count < 1 || thread_count > MAX_THREADS) {
				fprintf(stderr,
					"Pocet vlaken je omezen na rozsah 1 az %d\n",
					MAX_THREADS);
				exit(2);
			}
			break;
		// -t transactions per thread
		case 't':
			bank_transactions_per_thread = atoi(optarg);
			break;
		// -a deposited amount of money
		case 'a':
			bank_transaction_amount = atoll(optarg);
			break;
		// -f fee per transaction
		case 'f':
			bank_transaction_fee = atoll(optarg);
			break;
		// -s initial account balance
		case 's':
			account_balance = atoll(optarg);
			break;
		// be quiet
		case 'q':
			verbose = 0;
			break;
		// be verbose
		case 'v':
			verbose++;
			break;
		// wait for parallel start
		case 'w':
			sync_start = 1;
			break;
		// help
		case 'h':
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);
			break;
		// unknown switch
		default:
			fprintf(stderr, "%c: unknown option.\n", optopt);
			usage(stderr, argv[0]);
			exit(2);
			break;
		}
	}
}
