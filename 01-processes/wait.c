// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// wait(2), exit(3)
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void) {
	pid_t child;
	int child_stat;

	// fork a child process
	child = fork();
	if (child == -1) {	// error
		perror("fork");
		exit(11);
	}
	else
	if (child == 0) {	// child
		sleep(2);
		return 3;
	}
	else {			// parent
		wait(&child_stat);
		if (WIFEXITED(child_stat)) {
			printf("Child process terminated with exit code %d.\n",
				WEXITSTATUS(child_stat));
		}
	}	
	return 0;
}
