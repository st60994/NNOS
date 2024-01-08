// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// test global variables and errno changed in threads
// test globální proměných a errno měněných ve vláknech
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *hello = "Hello";
char *world = "world!";
int global = 0;
time_t start_time;

// wait for time change (a new second) -- at most one second
// čeká na změny času (na novou sekundu) -- nejvýše jednu sekundu
static void sync_threads(void)
{

	while (time(NULL) == start_time) {
		// do nothing except chew CPU slices for up to one second
		// nedělá nic než spotřebovává procesorový čas zjišťováním času
	}
}

// print the given string to stdout -- run as thread
// výpis řetězce předaného jako argument -- spouštěno jako vlákno
void *print_string(void *arg)
{
	char *s = (char *) arg;

	sync_threads();		// synchronize thread start / synchronizace startu vláken
	errno++;		// change of "global" variable errno / změna „globální“ proměnné errno
	global++;		// change of global variable / změna globální proměnné
	printf("%s (%d, %d) ", s, errno, global);	// print string and errno value
	fflush(stdout);
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread1;
	pthread_t thread2;
	int err;

	start_time = time(NULL);
	errno = 4;
	global = 0;
	printf("errno  = %d\n", errno);
	printf("global = %d\n", global);
	err = pthread_create(&thread1, NULL, print_string, hello);
	if (err) {
		fprintf(stderr, "pthread_create 1: %s\n", strerror(err));
	}
	// (void) pthread_join(thread1, NULL); // ensure output string order / zaručí pořadí výpisu řetězců

	err = pthread_create(&thread2, NULL, print_string, world);
	if (err) {
		fprintf(stderr, "pthread_create 2: %s\n", strerror(err));
	}

	(void) pthread_join(thread1, NULL);
	(void) pthread_join(thread2, NULL);

	// assuming that no error happened during library and/or system calls
	// and thus errno was not changed by those calls
	// předpokládá se, že se nestala chyba během knihovních anebo
	// systémových volání, a tudíž se v nich hodnota errno nezměnila
	printf("\n"
	       "errno  = %d\n", errno);	// check errno
	printf("global = %d\n", global);	// check global
	return EXIT_SUCCESS;
}
