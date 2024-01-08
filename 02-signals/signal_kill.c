// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// signal(2), kill(2)
// There is a problem in our solution for handling a signal:
//	It is possible that the handler is not invoked at all (even though the signal was delivered).
//	Why?
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int sig)
{
//	signal(sig, handler);	// install the signal handler again
	printf("The child process received the signal.\n");
}

int main()
{
	pid_t child;
	int child_stat;

	if ((child = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (child == 0) {		// child process
		// set the signal handler
		signal(SIGUSR1, handler);
		sleep(10);
		printf("The child process terminates.\n");
		exit(0);
	}
	else {				// parent process
		printf("Sending the signal to child PID %d.\n", child);
		if (kill(child, SIGUSR1) < 0)
			perror("kill");
	}
	wait(&child_stat);		// wait for child termination
	exit(0);
}
