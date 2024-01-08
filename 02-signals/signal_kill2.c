// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// signal(2), kill(2)
// Three signals can be handled only once. Why?
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
	printf("The child process received the signal #%d.\n", sig);
}

int main()
{
	pid_t child;
	int child_stat;

	signal(SIGUSR1, handler);	// set the signal handler
	if ((child = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (child == 0) {		// child process
		sleep(2);
		printf("The child process terminates.\n");
		exit(0);
	}
	else {				// parent process
		signal(SIGUSR1, SIG_DFL);	// reset the signal handler
		// send the signal two times
		printf("Sending the signal to child PID %d three times.\n", child);
		if (kill(child, SIGUSR1) < 0)
			perror("kill");
		if (kill(child, SIGUSR1) < 0)
			perror("kill");
		if (kill(child, SIGUSR1) < 0)
			perror("kill");
	}
	wait(&child_stat);		// wait for child termination
	exit(0);
}
