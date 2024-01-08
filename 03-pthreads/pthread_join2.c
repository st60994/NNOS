// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_join(3)
//
// partly correct way of passing the result from a thread (type casting pointer as integer)
// napůl správný způsob předávání výsledku z ukončeného vlákna (přetypování ukazatele na int)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>		// atoi(3)

// uncomment to use intptr_t
//#include <inttypes.h>		// intptr_t

// find n-th prime number / hledá n-té prvočíslo
void *compute_prime(void *arg)
{
	int candidate = 2;	// try to change int to intptr_t
	int n = *((int *) arg);

	if (n <= 0)
		return NULL;

	while (1) {
		int factor;
		int is_prime = 1;
		for (factor = 2; factor <= candidate / 2; ++factor)
			if (candidate % factor == 0) {
				is_prime = 0;
				break;
			}
		if (is_prime) {
			if (--n == 0)
				// type casting the int to pointer:
				// works only if the size of int is the same as pointer
				// přetypování int na ukazatel
				// funguje jen pokud má int stejnou velikost jako ukazatel
				return (void *)candidate;
		}
		++candidate;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	int prime = 0;		// try to change int to intptr_t
	int which_prime;

	if (argc < 2) {
		fprintf(stderr, "Argument missing (n number).\n"
			"Prints n-th prime number.\n");
		return EXIT_FAILURE;
	}
	which_prime = atoi(argv[1]);	// string -> number

	// calculation is done by a thread / výpočet provede vlákno
	pthread_create(&thread, NULL, compute_prime, &which_prime);

	// meanwhile, we can do something else / mezitím lze provádět něco jiného

	// wait for the result = wait for the thread termination
	// počkání na výsledek = čekání na ukončení vlákna
	pthread_join(thread, (void *)&prime);

	if (prime == 0)
		printf("Bad input.\n");
	else
		printf("Prime number %d is: %d\n", which_prime, prime);
		// for intptr_t replace the d in the second %d with PRIdPTR macro:
		// printf("Prime number %d is: %" PRIdPTR "\n", which_prime, prime);
	return EXIT_SUCCESS;
}
