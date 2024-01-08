// Operating Systems
// Processes
// Communication Using an Unnamed Socket
// fork(2), wait(2), socketpair(2)
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <ctype.h>

int main(void)
{
	int sv[2];		// the pair of socket descriptors
	char buf;		// buffer for data exchange between processes

	// create an unnamed socket pair of specified type:
	// two descriptors are stored into the sv array
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
		perror("socketpair");
		return EXIT_FAILURE;
	}

	switch (fork()) {
		case -1:	// error
			return EXIT_FAILURE;

		case 0:		// child
			close(sv[0]);		// close the other end of the connection
			read(sv[1], &buf, 1);	// read data from the socket
			printf("child:  read '%c'\n", buf);
			buf = toupper(buf);	// make it uppercase
			printf("child:  sent '%c'\n", buf);
			write(sv[1], &buf, 1);	// send data back
			break;
		default:	// parent
			close(sv[1]);		// close the other end of the connection
			printf("parent: sent 'b'\n");
			write(sv[0], "b", 1);	// send data to the socket
			read(sv[0], &buf, 1);	// read data
			printf("parent: read '%c'\n", buf);
			wait(NULL);		// wait for a child
	}

	return EXIT_SUCCESS;
}
