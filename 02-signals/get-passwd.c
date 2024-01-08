// Operating Systems: sample code
// Signals
#include <stdio.h>	// standard I/O functions
#include <unistd.h>	// standard Unix functions, like getpid()
#include <signal.h>	// signal name macros, and the signal() prototype
#include <stdlib.h>	// system()
#include <string.h>	// strcmp()

#define	GOOD_PASSWORD	"marvelous"	// password we're expecting to get

// define values for 'YES' and 'NO'
#ifndef	YES
#  define YES	1
#  define NO	0
#endif


// This function switches echo mode on or off. In off mode, nothing
// the user types is echoed on the screen (e.g. when typing a password).
// Note: We're using the 'stty' command to switch the mode. There are
//       cleaner ways, but we don't want to complicate things now.
void echo_on(int is_it)
{
	if (is_it == YES)
		system("stty echo");
	else
		system("stty -echo");
}

// Here is the signal handler. It switches the input mode to echo-on,
// then suspends the process using a STOP signal. Once the process'
// operation is resumed, the function continues right after the call to
// the kill() function, sets echo mode off, and reproduces the password
// prompt, so the user will know that we're still expecting a password.
void catch_suspend(int sig_num)
{
	signal(sig_num, catch_suspend);	// re-set the signal handler

	printf("\nSuspending execution...\n");
	fflush(stdout);

	echo_on(YES);			// re-enable echo mode

	kill(getpid(), SIGSTOP);	// stop self

	// we'll get back here when the process is resumed
	printf("Resuming execution...\n");

	echo_on(NO);			// disable echo mode again

	printf("Password: ");		// reproduce the prompt
	fflush(stdout);
}


// -------- and the main function goes here ---------

int main(int argc, char *argv[])
{
#define MAX_SIZE 30
	char user[MAX_SIZE];	// user name supplied by the user
	char passwd[MAX_SIZE];	// password supplied by the user

	printf("Username: ");		// prompt the user for a user name
	fflush(stdout);

	fgets(user, MAX_SIZE, stdin);	// wait for input

	signal(SIGTSTP, catch_suspend);	// set the TSTP (Ctrl-Z) signal handler

	printf("Password: ");		// prompt the user for a password
	fflush(stdout);

	echo_on(NO);			// set input to no-echo mode
	fgets(passwd, MAX_SIZE, stdin);	// get the user input
	echo_on(YES);			// re-enable echo on input

	printf("\n");			// the Enter pressed by the user was not echoed
	fflush(stdout);

	signal(SIGTSTP, SIG_DFL);	// switch the TSTP signal handler to its default behaviour

	// verify the password (\n is stored, don't compare it)
	if (strlen(passwd) - 1 == strlen(GOOD_PASSWORD) &&
	    strncmp(passwd, GOOD_PASSWORD, strlen(passwd) - 1) == 0)
		printf("Access granted.\n");
	else
		printf("Access denied.\n");

	return 0;
}
