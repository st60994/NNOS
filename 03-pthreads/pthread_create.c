// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_create(3)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// usleep(3)

int count = 1000;
 unsigned int sleep_time = 10;

void *print_c(void *unused)
{
	int i;

	// print the character count times / vypisuje znak count-krát
	for (i = 1; i <= count; i++) {
		fputc('.', stderr);
		usleep(sleep_time);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	pthread_t thread_id;

	// create another thread which calls / vytvoří další vlákno, které spustí:
	// print_c(NULL);
	pthread_create(&thread_id, NULL, print_c, NULL);	// no error checking for lucidity

	// print the character count times / vypisuje znak count-krát
	for (i = 1; i <= count; i++) {
		fputc ('o', stderr);
		usleep(sleep_time);
	}

	usleep(count*sleep_time);
	fputc('\n', stderr);

	return EXIT_SUCCESS;
}
