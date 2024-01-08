// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// fork(2), getpid(2), getppid(2)
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main(void) {
	pid_t child;

	child = fork();
	if (child == -1) {
		// fprintf(stderr, "fork failed: %s\n", strerror(errno));
		perror("fork");
		exit(11);
	}
	else
	if (child == 0) {	// only the child process
		printf("This is a child with PID %d, my parent's PID is %d.\n", getpid(), getppid());
	}
	else {			// only the parent process
		printf("This is a parent with PID %d, spawned a child with PID %d.\n", getpid(), child);
	}
	// both processes: parent and child
	sleep(10);
	return 0;
}
