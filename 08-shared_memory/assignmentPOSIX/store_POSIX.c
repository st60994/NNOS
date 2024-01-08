// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// POSIX:	shm_open(3), umask(2), ftruncate(2), mmap(2), munmap(2), shm_unlink(3), close(2)
// System V:	shmget(2), shmop(2), shmctl(2), ftok(3)

// Modified: 2022-12-06

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>			// permissions: S_I…
#include <fcntl.h>			// flags: O_…
#include <string.h>

// IPC:
#include <sys/mman.h>

#include "shmPOSIX.h"			// shared data structure

mem_texts_t *ptexts = MAP_FAILED;	// pointer to shared memory
char *shm_name = SHM_NAME;

int main(int argc, char *argv[])
{
	size_t length = 0;		// size of the shared memory mapping
	mode_t mode;			// permissions
	int i;
	char *pos;

	// check minimum number of arguments
	if (argc < 3) {
		fprintf(stderr, "Use:\n  %s mode text1 [text2 …]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// get the mode
	mode = strtol(argv[1], NULL, 0);
	mode &= 0777;			// restrict mode to 9 bits

	// set the storage size
	length = sizeof(((mem_texts_t *)0)->count);	// size of the count

	// add sizes of all text arguments

	fprintf(stderr, "Writing %zd bytes, %d texts.\n", length, argc - 2);

	// store individual arguments (including \0)
	pos = ptexts->texts;
	for (i = 2; i < argc; pos += strlen(argv[i++]) + 1)

	return EXIT_SUCCESS;
}
