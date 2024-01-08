// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// signal(2), kill(2), pause(2)
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int alarm_fired = 0;

// signal handler
void ding(int sig)
{
	alarm_fired = 1;
}

int main(void)
{
	pid_t pid;

	printf("Alarm application starting.\n");
	pid = fork();
	switch(pid) {
	case -1:	// error
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:		// child process
		sleep(10);
		// after 10 seconds send waking signal to the parent process
		if ((kill(getppid(), SIGALRM)) < 0)
			perror("(potomek) kill");
		exit(0);
	}
	// parent process
	signal(SIGALRM, ding);	// set the signal handler
	printf("PID %d: Waiting for alarm.\n", getpid());
	pause();		// sleep until a signal is received: see pause(2)
	if (alarm_fired)
		printf("Ding!\n");
	printf("Done\n");
	exit(EXIT_SUCCESS);
}
