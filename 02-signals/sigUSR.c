// Operating Systems: sample code  (c) Tomáš Hudec
// Signals
// kill(2), wait(2)
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <wait.h>

/*
   During the background execution (while the process sleeps for 10 s) run ps: 
   Spusťte program na pozadí a v době 10s čekání spusťte ps:

   ps -lH

   or / nebo

   ps -Ho state,pid,ppid,vsize,wchan,comm,args

   Watch states of the processes. / Sledujte stavy procesů.
*/

int main(int argc, char *argv[])
{
	int child_stat1, child_stat2;
	int child1, child2;

	printf("\nForking a child: sigUSR_a\n");
	if ((child1 = fork()) < 0)
		perror("fork");
	if (child1 == 0) {	// child 1
		if (execl("./sigUSR_a", "child_a", NULL) == -1) {
			perror("execl");
			return EXIT_FAILURE;
		}
	}

	printf("Forking a child: sigUSR_b\n");
	if ((child2 = fork()) < 0)
		perror("fork");
	if (child2 == 0) {	// child 2
		if (execl("./sigUSR_b", "child_b", NULL) == -1) {
			perror("execl");
			return EXIT_FAILURE;
		}
	}

	sleep(10);

	printf("\nSending signals to child processes.\n");
	if (kill(child1, SIGUSR1) == -1)
		perror("kill 1 failed");
	if (kill(child2, SIGUSR2) == -1)
		perror("kill 2 failed");

	sleep(10);

	printf("\nCalling wait.\n");
	wait(&child_stat1);
	wait(&child_stat2);

	printf("\nSleep before the end.\n");

	sleep(10);

	printf("\nFinished.\n");

	return EXIT_SUCCESS;
}
