// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// POSIX:	shm_open(3), umask(2), ftruncate(2), mmap(2), munmap(2), shm_unlink(3), close(2)
// System V:	shmget(2), shmop(2), shmctl(2), ftok(3)

// Modified: 2019-12-11

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>			// permissions: S_I…
#include <fcntl.h>			// flags: O_…
#include <string.h>

// IPC:
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shmSV.h"			// shared data structure

mem_texts_t *ptexts = ATTACH_FAILED;	// pointer to shared memory
int mem_id = -1;			// memory id

int main(int argc, char *argv[])
{
	struct shmid_ds shmds;
	int i;
	char *pos;

	// check minimum number of arguments
	if (argc < 2) {
		fprintf(stderr, "Use:\n  %s shm-id\n", argv[0]);
		return EXIT_FAILURE;
	}

	// print size info
	printf("Shared memory size: %zd.\n", shmds.shm_segsz);

	// print the number of texts
	printf("Reading %d text%s:\n", ptexts->count, ptexts->count > 1 ? "s" : "");

	// read texts
	pos = ptexts->texts;

	return EXIT_SUCCESS;
}
