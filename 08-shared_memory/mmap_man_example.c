/*
    The following program prints part of the file specified  in  its  first
    command-line  argument  to  standard  output.  The range of bytes to be
    printed is specified via offset and length values  in  the  second  and
    third  command-line arguments.  The program creates a memory mapping of
    the required pages of the file and then uses  write(2)  to  output  the
    desired bytes.
*/

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_errno(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
	char *addr;
	int fd;
	struct stat sb;
	off_t offset, pa_offset;
	size_t length;
	ssize_t s;

	if (argc < 3 || argc > 4) {
		fprintf(stderr, "Use:\n  %s file offset [length]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// otevřeme soubor z parametru
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		handle_errno("open");

	// získáme velikost
	if (fstat(fd, &sb) == -1)	// to obtain file size
		handle_errno("fstat");

	// offset z 2. parametru
	offset = atoi(argv[2]);
	// zaokrouhlení offsetu (dolů) na hranici stránky
	pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	// offset for mmap() must be page aligned

	// offset musí být menší než velikost souboru
	if (offset >= sb.st_size) {
		fprintf(stderr, "offset is past end of file\n");
		exit(EXIT_FAILURE);
	}

	// 3. (volitelný) parametr je délka
	if (argc == 4) {
		length = atoi(argv[3]);
		// offset + délka nesmí být za koncem souboru
		if (offset + length > sb.st_size)
			length = sb.st_size - offset;
		// can't display bytes past end of file
	}
	else	// no length arg ==> display to end of file
		length = sb.st_size - offset;

	// přimapování paměti dané délky (plus případné zaokrouhlení na hranici stránky)
	addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
		    MAP_PRIVATE, fd, pa_offset);
	// MAP_PRIVATE = copy-on-write mapping: změny se neprojeví v souboru
	if (addr == MAP_FAILED)
		handle_errno("mmap");

	// file descriptor lze zavřít, mapování zůstane zachováno
	close(fd);

	// výpis zvolené části obsahu souboru pouhým výpisem obsahu paměti
	s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
	if (s != length) {
		if (s == -1)
			handle_errno("write");

		fprintf(stderr, "partial write");
		exit(EXIT_FAILURE);
	}

	// odstranění mapování
	munmap(addr, length + offset - pa_offset);

	exit(EXIT_SUCCESS);
} // main

