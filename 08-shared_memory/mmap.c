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

#define SHM_NAME	"/shm:hudec"	// unique name

#define FOR_ALL		1		// permissions for all

#if FOR_ALL == 1
#	define	MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#else
#	define	MODE	(S_IRUSR | S_IWUSR)
#endif

long int *addr = MAP_FAILED;		// pointer to shared memory
size_t length = sizeof(*addr);		// size of the shared memory mapping
int mem_fd = -1;			// memory descriptor

// release memory mapping and remove the shared memory name
void release_resources(void)
{
	// close opened descriptor
	if (-1 != mem_fd) {
		close(mem_fd);		// no error checking
		mem_fd = -1;
	}

	// remove the shared memory object name regardless of allocating it
	if (-1 == shm_unlink(SHM_NAME))
		if (ENOENT != errno)	// ignore the non-existing error
			perror("shm_unlink");

	// unmap if mapped
	if (MAP_FAILED != addr) {
		if (-1 == munmap(addr, length))
			perror("munmap");
		else
			addr = MAP_FAILED;
	}
}

int main(int argc, char *argv[])
{
	mode_t mask;			// save the umask
	off_t offset = 0;		// offset from which it is mapped to virtual address space
	long int value = 1234;		// a value to store

	// get the value from argument if it was given
	if (argc > 1)
		value = strtol(argv[1], NULL, 0);

	atexit(release_resources);	// remove the shared memory upon exit

	// permissions are evaluated with regards to umask
	mask = umask(0);		// set umask to zero

	// create/open the shared memory object
	mem_fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, MODE);
	if (-1 == mem_fd) {
		perror("shm_open");
		return EXIT_FAILURE;
	}

	umask(mask);			// restore umask

	// set the shared memory size
	if (-1 == ftruncate(mem_fd, length)) {
		perror("ftruncate");
		return EXIT_FAILURE;
	}

	// map the shared memory to process' virtual address space
	addr = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, offset);
	if (MAP_FAILED == addr) {
		perror("mmap");
		return EXIT_FAILURE;
	}

	// after mapping, the memory descriptor is not needed anymore
	if (-1 == close(mem_fd)) {
		perror("close");
		return EXIT_FAILURE;
	}
	mem_fd = -1;

	// work with the shared memory – writing to it
	fprintf(stderr, "PID %5d: storing the value: %ld\n", getpid(), value);
	*addr = value;

	sleep(3);			// wait for a while

	// work with the shared memory – reading from it
	fprintf(stderr, "PID %5d: expected value:    %ld, value read: %ld\n", getpid(), value, *addr);

	return EXIT_SUCCESS;
}
