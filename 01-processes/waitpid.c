// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// waitpid(2)
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

int main(void)
{
	pid_t child_pid;
	pid_t wait_pid;
	int wait_status;

	child_pid = fork();
	if (child_pid == -1) {		// error
		perror("fork");
		exit(EXIT_FAILURE);
    	}
	else
	if (child_pid == 0) {		// child
		sleep(60);
		return 6;
    	}
	else {				// parent
		printf("\n"
			"Parent's PID: %d\n"
			"Child's PID:  %d\n"
			"Try some commands with the child process:\n"
			"  kill -STOP / -TSTP / -CONT / -INT / -QUIT %d\n", getppid(), child_pid, child_pid);
		do {
			// wait for the child's status change
			wait_pid = waitpid(child_pid, &wait_status, WUNTRACED | WCONTINUED);
			if (wait_pid == -1) {
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			if (WIFEXITED(wait_status))
				printf("\nChild process finished, exit status: %d\n",
					WEXITSTATUS(wait_status));
			else
			if (WIFSIGNALED(wait_status)) {
				printf("\nChild process was terminated by signal: %d\n",
					WTERMSIG(wait_status));
#ifdef WCOREDUMP
				printf("Core was%s dumped.\n",
					WCOREDUMP(wait_status) ? "" : " not");
#endif
			}
			else
			if (WIFSTOPPED(wait_status))
				printf("\nChild process was stopped by signal: %d\n",
					WSTOPSIG(wait_status));
			else
			if (WIFCONTINUED(wait_status))
				printf("\nChild process was resumed by signal SIGCONT: %d\n", SIGCONT);
			else {
				// never happens
				printf("\nChild process was terminated.\n");
				return EXIT_FAILURE;
			}
		} while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
	}
	return EXIT_SUCCESS;
}

