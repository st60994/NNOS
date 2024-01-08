// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// pthread_create(3), pthread_exit(3)
//
// Assignment / Zadání:
// 1. Comment out definition of DO_USLEEP.
// 2. Comment out definition of LEAVE_BY_RETURN.
// 3. a) Uncomment definition of LEAVE_BY_RETURN and
//    b) comment out definition of NO_JOIN.

#define DO_USLEEP		1	// 1. comment out this
#define LEAVE_BY_RETURN 	1	// 2. comment out this
#define NO_JOIN 		1	// 3. comment out this and uncomment previous line

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// usleep(3)

int count = 1000;
unsigned int sleep_time = 10;
char c1 = '.';
char c2 = 'o';

void *print_c(void *arg)
{
	int i;
	char c = *(char *)arg;

	// print the character count times / vypisuje znak count-krát
	for (i = 1; i <= count; i++) {
		fputc(c, stderr);
#ifdef DO_USLEEP
		usleep(sleep_time);
#endif
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;

	// create another thread which calls print_c(&c1);
	pthread_create(&thread_id, NULL, print_c, &c1);	// no error checking for lucidity
	// this thread continues by calling:
	print_c(&c2);

#ifdef DO_USLEEP
	usleep(count*sleep_time);	// final sleep
#endif
#ifndef NO_JOIN
	pthread_join(thread_id, NULL);	// wait for the thread finish
#endif
	fputc('\n', stderr);

#ifdef LEAVE_BY_RETURN
	return EXIT_SUCCESS;		// exit (process)
#else
	pthread_exit(NULL);		// exit (thread)
#endif
}
