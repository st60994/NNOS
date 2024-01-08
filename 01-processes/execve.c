// Operating Systems: sample code  (c) Tomáš Hudec
// Processes
// fork(2), execve(2), exec(3), abort(3)
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

// uncomment the following line for background execution:
 #define DO_FORK

int run(char *program, char *arg_list[]) {
#ifdef DO_FORK
	pid_t child;
#endif
	// environment variables
	char *envp[] = {
		"VARIABLE=value",	// common form
		"PATH=/usr/bin:/bin",
		"LANG=cs_CZ.UTF-8",
		NULL
	};

#ifdef DO_FORK
	child = fork();
	if (child != 0) {	// error (-1) or parent process (>0)
		fprintf(stderr,
			"Running self in background.\n"
			"Try:\n"
			"  ps -o pid,ppid,comm=PROGRAM -o args=ARGUMENTS\n"
		);
		return child;	// returned value is used as exit code (-1 or PID)
	}
	// child process continues
	execl("/bin/sleep", "spi", "10", NULL);		// different process name try “ps -l” and “ps -f”
	perror("exec");
	exit(EXIT_FAILURE);
#endif

	// exec family: replace the program in the process
// 	execve(program, arg_list, envp);		// system call
//	execv(program, arg_list);			// lib alternative
	execvp(program, arg_list);			// lib alternative

//	execle(program, "ls", "-la", NULL, envp);	// lib alternative
//	execl(program, "ls", "-la", NULL);		// lib alternative
//	execlp(program, "ls", "-la", NULL);		// lib alternative

	perror("exec");		// display error string in case of error
//	exit(EXIT_FAILURE);	// exit with error status
	abort();		// abort---cause abnormal program termination
}

int main(int argc, char *argv[]) {
	// arguments of command to be executed
	// arg_list[0] (argument on the zero index) is the process name
	// the NULL in the array indicates the end of argument list
	// Note that there is a difference between an empty string argument ("")
	// and the end of the argument list (NULL)
	char *arg_list[] = {
		"ls", "-la", NULL
	};

//	run("/bin/ls", arg_list);
	run("ls", arg_list);

	return 0;
}
