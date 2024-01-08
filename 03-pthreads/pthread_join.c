// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_join(3)
//
// incorrect way of passing the result from a thread (reference to invalid variable)
// nesprávný způsob předávání výsledku z ukončeného vlákna (odkaz na neplatnou proměnnou)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>		// atoi(3)

// find n-th prime number / hledá n-té prvočíslo
void *compute_prime(void *arg)
{
	int candidate = 2;
	int n = *((int *) arg);

	if (n <= 0)
		return NULL;

	while (1) {
		int factor;
		int is_prime = 1;
		for (factor = 2; factor <= candidate / 2; ++factor)
			if (candidate % factor == 0) {	// if divisible
				is_prime = 0;		// not a prime
				break;
			}
		if (is_prime) {
			if (--n == 0)
				return &candidate;
				// Reference is no longer valid after returning! Why?
				// Odkaz je po návratu z funkce neplatný! Proč?
		}
		++candidate;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	int *prime = NULL;
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

	if (prime == NULL || *prime == 0)
		printf("Bad input.\n");
	else
		printf("Prime number %d is: %d\n", which_prime, *prime);
	return EXIT_SUCCESS;
}
