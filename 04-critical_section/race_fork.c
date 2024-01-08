// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// show race condition on terminal output
//
// Assignment:
// 1. Comment out the definition of NO_OUTPUT_BUFFER.
// 2. a) Define again NO_OUTPUT_BUFFER and
//    b) uncomment the definition of DO_FLUSH_OUT.
#define NO_OUTPUT_BUFFER
// #define DO_FLUSH_OUT

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>

time_t start_time;

// wait for time change (a new second) -- at most one second
// čeká na změny času (na novou sekundu) -- nejvýše jednu sekundu
static void wait_for_next_second(void)
{
	while (time(NULL) == start_time) {
		// do nothing except chew CPU slices by getting current time for up to the next second
		// nedělá nic než spotřebovává procesorový čas zjišťováním času do následující sekundy
	}
}

static void char_at_a_time(char *str)
{
	char *ptr;
	int c;
	wait_for_next_second();	// sync
	for (ptr = str; (c = *ptr); ++ptr) {
		putc(c, stdout);
#ifdef DO_FLUSH_OUT
		fflush(stdout);	// alternate way to avoid output buffering: force buffer to flush
#endif
		usleep(100);	// simulate slow terminal output
	}
}

int main(void)
{
	pid_t pid;

	start_time = time(NULL);

	// ensure that characters sent to stdout are output as soon as possible:
	// zajištění výpisu znaků na terminál ihned, bez výstupního bufferu
#ifdef NO_OUTPUT_BUFFER
	setbuf(stdout, NULL);	// make stdout unbuffered
#endif

	if ((pid = fork()) < 0) {
		perror("fork error");
		exit(1);
	}
	else if (pid == 0) {
		char_at_a_time("Output from a child.\n");
	}
	else {
		char_at_a_time("Output from the parent.\n");
		wait(NULL);
	}

	return EXIT_SUCCESS;
}
