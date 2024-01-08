// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// waitpid(2), strsignal(3)
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>

// SIGCHLD handler
void catch_child(int sig)
{
	int child_stat;
	pid_t pid;

	printf("Catched SIGCHLD: Calling waitpid().\n");
	// sleep(1);
	// WNOHANG -- non-blocking call -- returns immediately (0) if no child has been terminated
	while ((pid = waitpid(-1, &child_stat, WNOHANG)) > 0) {
		printf("Child PID %d terminated ", pid);
		if (WIFEXITED(child_stat))
			printf("with exit status %d.\n", WEXITSTATUS(child_stat));
		else if (WIFSIGNALED(child_stat))
			printf("by signal %d.\n", WTERMSIG(child_stat));
		else
			printf("abnormally.\n");
	}
}

int main(int argc, char *argv[])
{
	int child1, child2;
	unsigned int seconds_left;

	struct sigaction sa_chld;
	struct sigaction sa_old;

	memset(&sa_chld, 0, sizeof(sa_chld));
	sa_chld.sa_handler = catch_child;
	sigemptyset(&sa_chld.sa_mask);
	// blok othet SIGCHLD signals while handling it
	sigaddset(&sa_chld.sa_mask, SIGCHLD);
	// SIGCHLD is sent only if the child terminates (not if the child is stopped/resumed)
	sa_chld.sa_flags = SA_NOCLDSTOP | SA_RESTART;

	sigaction(SIGCHLD, &sa_chld, &sa_old);	// set signal handler

	// forking child 1
	printf("\nForking a child: sigUSR_a\n");
	if ((child1 = fork()) < 0)
		perror("fork");
	if (child1 == 0) {	// child 1
		printf("Child 1: PID %d.\n", getpid());
		if (execl("./sigUSR_a", "sigUSR_a", NULL) == -1) {
			perror("execl");
			exit(EXIT_FAILURE);
		}
	}
	// forking child 2
	printf("Forking a child: sigUSR_b\n");
	if ((child2 = fork()) < 0)
		perror("fork");
	if (child2 == 0) {	/* potomek 2 */
		printf("Child 2: PID %d.\n", getpid());
		if (execlp("./sigUSR_b", "sigUSR_b", NULL) == -1) {
			perror("execlp");
			exit(EXIT_FAILURE);
		}
	}

	printf("\n");
	sleep(2);

	// Sending signals to child processes
	printf("Report child 1!\n");
	if (kill(child1, SIGUSR1) == -1)
		perror("kill 1 failed");
	// sleep(1);
	printf("Report child 2!\n");
	if (kill(child2, SIGUSR2) == -1)
		perror("kill 2 failed");

	// wait
	// sleep is terminated after timeout OR upon receiving signal which is not ignored
	seconds_left = 10;
	while ((seconds_left = sleep(seconds_left)) > 0)
		// do nothing -- waiting 10 seconds
		;
	printf("Finished.\n");
	exit(0);
}
