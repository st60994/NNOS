// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// POSIX:	shm_open(3), umask(2), ftruncate(2), mmap(2), munmap(2), shm_unlink(3), close(2)
// System V:	shmget(2), shmop(2), shmctl(2), ftok(3)

// Modified: 2018-12-13

// IPC:
#include <sys/mman.h>			// shm_open(3), mmap(2), …

#include <sys/types.h>
#include <sys/stat.h>			// permissions: S_I…
#include <fcntl.h>			// flags: O_…
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

long int *addr = MAP_FAILED;		// pointer to shared memory
size_t length = sizeof(*addr);		// size of the shared memory mapping
int shmid = -1;				// memory descriptor

#define KEY         21   		// defined key


// release memory mapping and remove the shared memory name
void release_resources(void)
{
	// remove the shared memory object name regardless of allocating it
	if (-1 == shmctl(shmid, IPC_RMID, NULL))
		if (ENOENT != errno)	// ignore the non-existing error
			perror("shmctl");

	// unmap if mapped
	if (MAP_FAILED != addr) {
		if (-1 == shmdt(addr))
			perror("shmdt");
		else
			addr = MAP_FAILED;
	}
}

int main(int argc, char *argv[])
{
	mode_t mask;			// save the umask
	long int value = 1234;		// a value to store

	// get the value from argument if it was given
	if (argc > 1)
		value = strtol(argv[1], NULL, 0);

	atexit(release_resources);	// remove the shared memory upon exit

	// permissions are evaluated with regards to umask
	mask = umask(0);		// set umask to zero

	// create/open the shared memory object
	shmid = shmget(KEY, length, IPC_CREAT | 0666);
	if (-1 == shmid) {
		perror("shmget");
		return EXIT_FAILURE;
	}

	umask(mask);			// restore umask

	// map the shared memory to process' virtual address space
	addr = shmat(shmid, NULL, 0);
	if (MAP_FAILED == addr) {
		perror("shmat");
		return EXIT_FAILURE;
	}

	// work with the shared memory – writing to it
	fprintf(stderr, "PID %5d: storing the value: %ld\n", getpid(), value);
	*addr = value;

	sleep(3);			// wait for a while

	// work with the shared memory – reading from it
	fprintf(stderr, "PID %5d: expected value:    %ld, value read: %ld\n", getpid(), value, *addr);

	return EXIT_SUCCESS;
}
