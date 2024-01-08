// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// result passing: multiple versions in one file
// předávání výsledku: několik verzí v jedné
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

// conditional compilation, try changing / podmíněný překlad, zkuste změnit:
#define PRINT_TYPE	0	// 1. change to 1
#define ALLOC_THREAD	0	// 2. change to 1
#define VAR_TYPE	0	// 3. change to 1 and 2

#define BUFFER_SIZE	100	// buffer size

char result[BUFFER_SIZE];	// string buffer (for the thread return value)

void *compute_result(void *arg);	// computing thread function
void *just_allocate_mem(void *arg);	// help thread function

int main(int argc, char *argv[])
{
	pthread_t a_thread;
	long int arg = 47806;
	char *ret_str;

	if (argc > 1)	// at least one arg is given / nejméně jeden argument byl zadán
		arg = strtol(argv[1], NULL, 0);

	// create a thread
	errno = pthread_create(&a_thread, NULL, compute_result, &arg);
	if (errno) {
		perror("pthread_create failed");
		return EXIT_FAILURE;
	}

	// get the return value (string)
	errno = pthread_join(a_thread, (void *)&ret_str);
	if (errno) {
		perror("pthread_join failed");
		return EXIT_FAILURE;
	}

#if ALLOC_THREAD != 0
	// create a thread which only uses (overwrites) some memory
	// vytvoření vlákna, které pouze použije (přepíše) nějakou paměť
	errno = pthread_create(&a_thread, NULL, just_allocate_mem, NULL);
#if 0
	// errno = pthread_detach(a_thread);
#else
	errno = pthread_join(a_thread, NULL);
#endif
#endif

	// print the string returned by the thread
	// výpis řetězce vráceného vláknem
	printf("Returned string at the address 0x%08lX:\n  %s\n",
		(unsigned long)ret_str, ret_str);

	return EXIT_SUCCESS;
}

void *compute_result(void *arg)
{
	long int val = *(long int *)arg;	// dereferencing the argument
	int rc;
#if VAR_TYPE == 0
	char result[BUFFER_SIZE];		// string buffer: local, stack
#elif VAR_TYPE == 1
	char *result = malloc(BUFFER_SIZE);	// string buffer: dynamic, heap
#else
	// string buffer: static global variable, data
#endif

	// put the string into the result variable:
	rc = snprintf(result, BUFFER_SIZE,
#if PRINT_TYPE != 0
		"Given vaule of %ld is in hexadecimal base equal to "
#else
		"hexadecimal %ld = "
#endif
		"0x%lX", val, val);
	if (rc >= BUFFER_SIZE) {
		// insufficient buffer size
		fprintf(stderr, "Buffer overflow would occur, needed: %d, have: %d.\n",
			rc, BUFFER_SIZE);
		exit(EXIT_FAILURE);	// terminate the process (all threads)
	}

	pthread_exit(result);		// return value is the string in the result variable
}

// this thread uses only some memory on its stack
// and overwrites it
void *just_allocate_mem(void *arg)
{
	int i;
	char data[126-32+2];
	// printable ASCII characters from # 32, end of line (\n) and null (\0)
	for (i = 0; i < sizeof(data)-2; i++)
		data[i] = (char)(i+32);
	data[sizeof(data)-2] = '\n';	// new line
	data[sizeof(data)-1] = 0;	// C-string termination
	return NULL;
}

// memory of a process
// 
// ┏━━━━━━━┓ max address
// ┃ stack ┃ main thread stack
// ┠───↓───┨
// ┃       ┃
// ┠───────┨
// ┃ stack ┃ another thread stack
// ┠───↓───┨
// ┃       ┃
// ╹---⚡---╹
// ╻---⚡---╻
// ┃       ┃
// ┠───↑───┨
// ┃ heap  ┃ dynamic data
// ┠───────┨
// ┃ data  ┃ static data
// ┠───────┨
// ┃ code  ┃
// ┗━━━━━━━┛ min address (0)
