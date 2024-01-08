// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Example code that simulates several concurrent sequences of bank transactions.
// The critical section is the money deposit.
//
// Vkládání peněz na bankovní účet několika vlákny současně.
//
// Last modification: 2016-11-22, 2017-04-19, 2023-03-29

#if !defined _XOPEN_SOURCE || _XOPEN_SOURCE < 600
#	define _XOPEN_SOURCE 600	// enable barriers
#endif


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		// getopt
#include <errno.h>
#include <stdbool.h>

#define MAX_THREADS			100

static int sync_start = 0;	// synchronous start
pthread_barrier_t sync_start_barrier;
bool sync_start_barrier_initialized = false;

int verbose = 1;		// 0 = quiet mode

// default values
#define ACCOUNT_START			1
#define BANK_TRANSACTION_AMOUNT		101
#define BANK_TRANSACTION_FEE		1
#define BANK_TRANSACTIONS_PER_THREAD	10000
#define THREAD_COUNT			3

// initial account balance / stav účtu na začátku
volatile long long int account_balance = ACCOUNT_START;

// deposit value / částka pro vklad
static long long int bank_transaction_amount = BANK_TRANSACTION_AMOUNT;

// transaction fee / poplatek za převod
static long long int bank_transaction_fee = BANK_TRANSACTION_FEE;

// number of transactions per thread / počet transakcí na vlákno
static unsigned int bank_transactions_per_thread =
    BANK_TRANSACTIONS_PER_THREAD;

// number of threads / počet vláken
static unsigned int thread_count = THREAD_COUNT;


// structure array for threads / pole záznamů pro vlákna
struct thread_info_t {
	pthread_t id;
	int arg;
} thread_info[MAX_THREADS];

// prototypes
void eval_args(int argc, char *argv[]);

// variable declarations and function definitions for critical section access
// deklarace proměnných a definice funkcí pro ošetření kritické sekce



// release all allocated resources, used in atexit(3)
// uvolnění všech alokovaných prostředků, použito pomocí atexit(3)
void release_resources(void)
{       
	if (sync_start_barrier_initialized) {
		// destroy the barrier
		//if (DESTROY FAILED)
		//	perror("…");
		//else
		//	sync_start_barrier_initialized = false;
	}
}

// bank transaction: log, calculate, return the new balance
// bankovni operace: uložení záznamu, vyppočtení a vrácení nového stavu
long long int deposit_money(long long int value)
{
	// log the transaction / vytvoř záznam o operaci
	if (verbose > 2)
		printf("before: %lld, credit: %lld, fee: %lld\n",
			account_balance, value, bank_transaction_fee);
	// do calcutation and return the new account balance
	// provedení výpočtu a vrácení nového stavu účtu
	return account_balance + value - bank_transaction_fee;
}


// thread function for transactions / vláknová funkce pro provádění transakcí
// the argument is a pointer to the thread id
void *bank_deposit(void *arg)
{
	int i;
	int id = *(int *) arg;

	if (sync_start)
		// if requested, start synchronously
		switch ((errno = pthread_barrier_wait(&sync_start_barrier))) {
			case PTHREAD_BARRIER_SERIAL_THREAD:
			      if (verbose)
			      	printf("The threads have started transactions.\n");
			case 0:
				break;
			default:
				perror("pthread_barrier_wait");
				exit(3);
		}

	if (verbose > 1)
		printf("The thread %d has started transactions.\n", id);

	// perform several transactions per thread
	for (i = 0; i < bank_transactions_per_thread; i++) {

		// save the new account balance / ulož nový stav účtu
		account_balance = deposit_money(bank_transaction_amount);

	}

	if (verbose > 1)
		printf("The thread %d has finished transactions.\n", id);

	return NULL;
}


//
// main / hlavní program
//
int main(int argc, char *argv[])
{
	int i;
	long long int expected;		// očekavaný stav

	// argument(s) evaluation / zpracování argumentů
	eval_args(argc, argv);

	// calculate the expected account balance after all transactions
	// spočti očekávaný stav účtu po provedení všech tranaskcí
	expected =
	    account_balance +
	    thread_count * bank_transactions_per_thread *
		(bank_transaction_amount - bank_transaction_fee);

	// init for the critical section access control
	// inicializace pro ošetření kritické sekce


	// barrier initialization
	if (sync_start) {
		if ((errno = pthread_barrier_init(&sync_start_barrier, NULL, thread_count))) {
			perror("pthread_barrier_init");
			return 3;
		}
		sync_start_barrier_initialized = true;
	}

	if (verbose)
		printf("Balance before: %10lld\n", account_balance);

	// start several threads performing transactions
	for (i = 0; i < thread_count; i++) {
		thread_info[i].arg = i;
		// run a new transaction thread
		if ((errno = pthread_create(&thread_info[i].id, NULL, bank_deposit, &thread_info[i].arg))) {
			// failed to create a thread
			perror("pthread_create");
			return 3;
		}
	}

	// wait for threads to finish / čekej na dokončení vláken
	for (i = 0; i < thread_count; i++) {
		pthread_join(thread_info[i].id, NULL);
	}

	if (verbose)
		printf("Balance after:  %10lld\n", account_balance);

	// compare the result with expected value / porovnej výsledek s očekávanou hodnotou
	if (account_balance != expected) {
		fprintf(stderr,
			"ERROR IN TRANSACTIONS! Balance: %lld, expected: %lld\n",
			account_balance, expected);
		return 1;
	}

	return 0;
}


//
// usage
//
void usage(FILE * stream, char *self)
{
	fprintf(stream,
		"Usage:\n"
		"  %s -h\n"
		"  %s [-q|-v] [-w] [-c threads] [-t tansactions] [-a amount] [-s init_state]\n"
		"Purpose:\n"
		"  Simulation of concurrent bank transactions.\n"
		"Options:\n"
		"  -h		help\n"
		"  -c count	the number of concurrent threads (%u, max. %d)\n"
		"  -t count	the number of transactions per one thread (%u)\n"
		"  -a amount	the transaction deposit value (%lld)\n"
		"  -f amount	the transaction fee (%lld)\n"
		"  -s state	the initial account balance (%lld)\n"
		"  -w		wait for synchronous start of all transaction threads\n"
		"  -q		do not print account balance state\n"
		"  -v		print more verbose information\n",
		self, self,
		thread_count, MAX_THREADS,
		bank_transactions_per_thread,
		bank_transaction_amount,
		bank_transaction_fee,
		account_balance);
}


//
// arguments and switches evaluation / vyhodnocení přepínačů z příkazové řádky
//
void eval_args(int argc, char *argv[])
{
	int opt;		// option

	opterr = 0;		// do not print errors, we'll print it
	// switches processing
	while ((opt = getopt(argc, argv, "hwqvc:t:a:f:s:")) != -1) {
		switch (opt) {
		// -c thread_count
		case 'c':
			thread_count = atoi(optarg);
			if (thread_count < 1 || thread_count > MAX_THREADS) {
				fprintf(stderr,
					"The number of threads is limited to 1 to %d\n",
					MAX_THREADS);
				exit(2);
			}
			break;
		// -t transactions_per_thread
		case 't':
			bank_transactions_per_thread = atoi(optarg);
			break;
		// -a transaction_amount
		case 'a':
			bank_transaction_amount = atoll(optarg);
			break;
		// -f transaction_fee
		case 'f':
			bank_transaction_fee = atoll(optarg);
			break;
		// -s initial_account_balance
		case 's':
			account_balance = atoll(optarg);
			break;
		// quiet
		case 'q':
			verbose = 0;
			break;
		// more verbose
		case 'v':
			verbose++;
			break;
		// wait for synchronous start
		case 'w':
			sync_start = 1;
			break;
		// help
		case 'h':
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);
			break;
		// unknown option
		default:
			fprintf(stderr, "%c: unknown option.\n", optopt);
			usage(stderr, argv[0]);
			exit(2);
			break;
		}
	}
}

// EOF
