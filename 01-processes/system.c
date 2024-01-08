// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// system(3)
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void) {
	int status;

	// library call system(3): execute a shell with given command
//	status = system("ls -la");
//	try:
//	status = system("ls -la; exit 126");
//	status = system("ls -la; false");
//	status = system("neexistuje");
//	status = system("ls -la; exit 127");
	status = system("ls -la; kill $$");

	if (status == -1) {
		// error
		perror("system");
		exit(1);
	}
	if (WIFEXITED(status)) {
		// proces terminated using exit / command not found
		switch (WEXITSTATUS(status)) {
			case 127:
				fprintf(stderr, "Couldn't run shell or command not found.\n");
				exit(127);
			default:
				printf("Process exit status: %d\n", WEXITSTATUS(status));
				exit(0);
		}
	}
	else {
		// proces terminated by signal
		printf("Process terminated by signal: %d\n", WTERMSIG(status));
	}

	return 0;
}
