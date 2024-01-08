// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Critical Sections
//
// Modified: 2015-11-11, 2016-11-14, 2017-04-18, 2023-03-29
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define THREADS 2		// the number of threads
#define ITERATIONS 1000000	// the number of operations per thread

volatile int count = 0;		// shared variable

void *ThreadAdd(void *arg)
{
	int i;
	// register int reg;

	for (i = 0; i < ITERATIONS; ++i) {
		// Compilation of "count++" is platform (CPU) dependent because
		// the CPU can store the value into register, increase that register
		// and store the result back into the memory variable.
		// We can simulate this behavior by three commands with a local variable.
		count++;		// usually compiled using a CPU register:
		// reg = count;		// save the global count locally
		// reg += 1;		// increment the local copy
		// count = reg;		// store the local value into the global one
		// You can check dissassembly using objdump:
		//	objdump -M intel --disassemble=ThreadAdd thread_add
		// or
		//	gdb -batch -ex 'disassemble/m ThreadAdd' thread_add
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid[THREADS];
	int t;

	// create threads
	for (t = 0; t < THREADS; ++t)
		if ((errno = pthread_create(&tid[t], NULL, ThreadAdd, NULL))) {
			perror("pthread_create failed");
			return EXIT_FAILURE;
		}

	// wait for all threads termination
	for (t = 0; t < THREADS; ++t)
		if ((errno = pthread_join(tid[t], NULL))) {
			perror("pthread_join failed");
			return EXIT_FAILURE;
		}

	// check the result
	if (count < THREADS * ITERATIONS) {
		fprintf(stderr, "BOOM! count is %d, should be %d\n", count, THREADS * ITERATIONS);
		return EXIT_FAILURE;
	}
	else {
		printf("OK! count is %d\n", count);
		return EXIT_SUCCESS;
	}
}

// EOF
