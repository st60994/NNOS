// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// signal(2), raise(3)
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>

void handler(int sig)
{
	printf("SigUSR_b reporting.\n");
	sleep(2);
	printf("SigUSR_b terminating.\n");
	raise(SIGTERM);			// equivalent to: kill(getpid(), SIGTERM);
	// exit(22);
}

int main(int argc, char *argv[])
{
	signal(SIGUSR2, handler);	// set handler
	pause();			// wait for a signal
	return 1;
}
