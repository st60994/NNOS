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
	struct stat info;
	int i;
	char *pos;

	// use given shm name
	if (argc > 1)
		shm_name = argv[1];

	// print size info
	printf("Shared memory size: %zd.\n", info.st_size);

	// print the number of texts
	printf("Reading %d text%s:\n", ptexts->count, ptexts->count > 1 ? "s" : "");

	// read texts
	pos = ptexts->texts;

	return EXIT_SUCCESS;
}
