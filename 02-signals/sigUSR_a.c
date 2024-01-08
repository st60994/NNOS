// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// signal(2)
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>

void handler(int sig)
{
	printf("SigUSR_a reporting.\n");
	sleep(1);
	printf("SigUSR_a terminating.\n");
	exit(11);
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, handler);	// set handler
	pause();			// wait for a signal
	return 1;
}
