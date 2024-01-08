// Operating Systems
// Processes
// fork(2), exit(3), wait(2), pipe(2)
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define PIPE_BUF (1<<8)		// buffer size
int fd[2];			// file descriptors for pipe(2)

void child()
{
	int r;
	char buf[PIPE_BUF];

	printf("child: This is a child!\n");
	printf("child: Reading message from my parent.\n"
	       "child: From pipe:\n");
	fflush(stdout);

	// reading from the pipe / čtení z roury
	while ((r = read(fd[0], buf, PIPE_BUF)) > 0) {
		write(STDOUT_FILENO, "  ", 2);	// print prefix
		write(STDOUT_FILENO, buf, r);	// print message
	}
	close(fd[0]);		// close the pipe / uzavření roury

	printf("child: My parent closed the pipe, exiting.\n");

	exit(EXIT_SUCCESS);
}

void parent()
{
	char *msg1 = "to pipe: Your name is Louie, my child.\n";
	char *msg2 = "to pipe: Now return.\n";

	write(fd[1], msg1, strlen(msg1));	// write to the pipe
	sleep(1);
	write(fd[1], msg2, strlen(msg2));	// write to the pipe
	sleep(1);
	printf("parent: Closing the pipe.\n");
	close(fd[1]);				// close the pipe

	wait(NULL);			// wait for child process termination

	printf("parent: The child process terminated.\n");
}

int main(int argc, char *argv[])
{
	int pid;

	pipe(fd);		// create a pipe (one way communication)
	// fd is pointer to array of two descriptors:
	// fd[0]: reading from the pipe
	// fd[1]: writing to the pipe

	printf("parent: Forking a child.\n");
	// fork(2) duplicates the process, which differs only by PID
	// all opened file descriptors are duplicated (including fd[0] and fd[1])
	// return value differs
	switch (pid = fork()) {
	case -1:		// error
		perror("fork(2) call failed");
		exit(EXIT_FAILURE);	// exit in case of error

	case 0:			// child process
		close(fd[1]);	// close writing end of the pipe
		child();
		break;
	default:		// parent
		close(fd[0]);	// close reading end of the pipe
		parent();
		break;
	}
	return EXIT_SUCCESS;	// return in main() is equivalent to exit()
}
