/*

Paging: Address Translation. / Stránkování: Převod adresy.
(c) Tomáš Hudec 2012

Zadání modifikováno: 2012-11-28

Stručné zadání:
Program dostane v parametru lineární (virtuální) adresu
a má ji pomocí dané stránkové tabulky převést na fyzickou.
Stránková tabulka je v souboru a ten je třeba do paměti
načíst pomocí mmap(2).

Podrobněji:
Zadaná lineární adresa má vždy velikost dle LINEAR_ADDRESS_SIZE (32 bitů).
Přepínači lze ovlivnit:
	kolik bitů adresy tvoří offset (-o),
	kolik bitů má položka stránkové tabulky (-e),
	jméno souboru se stránkovou tabulkou (-t) a
	nejvýše kolik bitů se použije pro číslo rámce (-f).

Uvažuje se, že formát položky stránkové tabulky je:
	+------------------+-------------+-------------+
	| rezervované bity | číslo rámce | řídicí bity |
	+------------------+-------------+-------------+
Nejnižší bity jsou řídicí, přičemž jich je tolik, kolik je rozsah offsetu.
Je-li tedy pro offset vyhrazeno 12 bitů, řídicích bitů v položce
stránkové tabulky je taktéž 12.
Přepínačem -f lze určit, jsou-li nějaké bity v položce rezervované,
resp. se jím určí, kolik bitů má číslo rámce. Bez přepínače -f nejsou
žádné bity rezervované a číslo rámce zabere zbytek položky stránkové tabulky.

Stránková tabulka je daná souborem (lze ovlivnit přepínačem -t).
Pro zvýšení efektivity je třeba načíst soubor se stránkovou tabulkou
do paměti pomocí systémového volání mmap(2).

*/

#include <errno.h>		// errno
#include <sys/types.h>		// system types
#include <inttypes.h>		// standard types of int: uint32_t etc.
#include <stdlib.h>		// exit, strtol
#include <stdio.h>		// standard io: printf
#include <sys/stat.h>		// fstat
#include <fcntl.h>		// open
#include <unistd.h>		// getopt

#include <sys/mman.h>		// file loading via mmap


// implicit values
#define DEFAULT_VERBOSITY		1	// print some messages

#define LINEAR_ADDRESS_SIZE		32	// bits

#define DEFAULT_PT_FNAME		"page_table.dat"

#define DEFAULT_OFFSET_SIZE		12	// bits
#define DEFAULT_PAGE_TABLE_ENTRY_SIZE	32	// bits

int verbose = DEFAULT_VERBOSITY;		// verbosity

uint8_t offset_size = DEFAULT_OFFSET_SIZE;	// offset size in bits
uint8_t page_table_entry_size = DEFAULT_PAGE_TABLE_ENTRY_SIZE; 	// page table entry size in bits
uint8_t frame_start_bit = DEFAULT_OFFSET_SIZE;	// frame's least significant bit in PT entry
uint8_t frame_size_limit = 64;			// max frame size limit in bits
char *pt_filename = DEFAULT_PT_FNAME;		// name of a file containing page table
uint8_t frame_size;				// frame size in bits
						// (derived from page table entry size and offset)

#if LINEAR_ADDRESS_SIZE <= 32
typedef uint32_t linear_t; 			// linear address type for address size 32 bits
#else
typedef uint64_t linear_t; 			// linear address type for 64-bit size
#endif
typedef uint64_t physical_t; 			// physical address type (always 64 bits)

// prototype for evaluating options function
void eval_args(int argc, char *argv[]);

// Translate page number to frame address using given page table
// Return value:
#define SUCCESS			0		// no error
#define INVALID_PAGE_SIZE	1		// page size (in bits) is invalid, must be <= 64
#define INVALID_PAGE_NUMBER	2		// given page number is larger than allowed
int page2frame(
	linear_t page,				// page number (index to the page table)
	uint8_t page_bits,			// number of bits used in page numbers
						// this also defines number of page table entries
	void *page_table,			// pointer to the page table
	const uint8_t page_table_entry_size,	// page table entry size
						// together with page_bits it defines page table size
	physical_t *frame			// returned frame address (offset bits are zeroed)
)
{
	// check page bits
	if (page_bits > 64)
		return INVALID_PAGE_SIZE;

	// page out of limits
	if (page >= (1 << page_bits))
		return INVALID_PAGE_NUMBER;

	// get the page table entry using given page number as an index
	//
	// GCC: 6.22 Arithmetic on void- and Function-Pointers:
	// Addition and subtraction operations are supported on pointers to void
	// and on pointers to functions.
	// This is done by treating the size of a void or of a function as 1. 
	//
	// Therefore we can reach the desired page table entry
	// by adding to the page table pointer.
	
	// FIXME

	return SUCCESS;
}

// Address translation from linear address apace to physical address space.
// Input parameters:
// 	linear address,
// 	offset size in bits,
// 	pointer to a page table,
// 	number of page table entries.
// Output parameters:
// 	physical address.
// Return value:
// 	0 = success
int linear2physical(
	const linear_t linear_address,		// given linear address
       	uint8_t offset_size,			// offset size in bits
	void *page_table,			// page table pointer
	const uint8_t page_table_entry_size,	// page table entry size
	physical_t *physical_address		// returned physical address
)
{
	linear_t page_number;		// page number
	linear_t offset;		// relative address in a page (offset)
	int ret = SUCCESS;		// return value

	// print input: linear address
	if (verbose > 0)
		printf("Linear address:   0x%08X\n", linear_address);

	// FIXME

	// print page number and offset
	if (verbose > 0)
		printf("Page number:      0x%08X, offset: 0x%0X\n",
			page_number, offset);

	// FIXME ret = page2frame(page_number, …, physical_address);
	if (SUCCESS != ret)
		return ret;

	// print frame number / frame address
	if (verbose > 0)
		printf("Frame number:     0x%08"PRIX64"\n", *physical_address >> offset_size);
	if (verbose > 1)
		printf("Frame address:    0x%08"PRIX64"\n", *physical_address);

	// FIXME physical address completion

	// print the physical address
	if (verbose > 1)
		printf("Physical address: 0x%08"PRIX64"\n", *physical_address);

	return ret;
}

// Load page table(s) in memory.
// Input parameters:
// 	file name (containing page table),
// Output parameters:
// 	address of the loaded page table,
// 	page table size.
// Return value:
// 	0 = success
int load_page_tables(char *fname, void **page_table, size_t *pt_size)
{
	int fd;		// file descriptor
	struct stat sb;	// stat structure (to get file size)

	// FIXME set page table size

	// open the file fname
	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return errno;
	}

	// stat the file, get the file size
	if (fstat(fd, &sb) == -1) {
		perror("fstat");
		return errno;
	}

	// check the size of the file
	if (*pt_size > sb.st_size) {
		fprintf(stderr, "%s: File size (0x%0lX bytes) is less than needed (0x%0zX bytes).\n",
				fname, sb.st_size, *pt_size);
		close(fd);
		return EXIT_FAILURE;
	}
	// print warning on larger file
	if (*pt_size < sb.st_size) {
		if (verbose > 0)
			fprintf(stderr, "%s: Warning, the file is larger (0x%0lX bytes) than needed (0x%0zX bytes).\n",
				fname, sb.st_size, *pt_size);
	}

	// FIXME

	return EXIT_SUCCESS;
}

// main program
int main(int argc, char *argv[])
{
	int i;
	int rc;
	uint32_t linear_address = 0;	// virtual address
	uint64_t physical_address = 0;	// physical address
	void *page_table;		// page table pointer
	size_t page_table_size;		// page table size

	// option processing, also gets pt_filename
	eval_args(argc, argv);

	rc = load_page_tables(pt_filename, &page_table, &page_table_size);
	if (rc != EXIT_SUCCESS) {
		fprintf(stderr, "%s: Error loading the file.\n", pt_filename);
		return EXIT_FAILURE;
	}

	// print page table info
	if (verbose > 0) {
		printf(	"Linear address size:   %d bits,\n"
			"offset size:           %d bits,\n"
			"page table entry size: %d bits,\n"
			"frame number size:     %d bits\n",
			LINEAR_ADDRESS_SIZE,
			offset_size,
		       	page_table_entry_size,
			frame_size);
	}

	if (optind == argc)
		fprintf(stderr, "Warning: No arguments given. Use -h for help.\n");

	// for every given linear address translate it to physical
	for (i = optind; i < argc; ++i) {
		linear_address = strtol(argv[i], NULL, 0);

		// translate
		if ((rc = linear2physical(linear_address, offset_size,
				page_table, page_table_entry_size, &physical_address)) != SUCCESS) {
			// error
			fprintf(stderr, "0x%08X: Error #%d translating given address.\n", linear_address, rc);
			continue;
		}
		
		// print the result -- physical address
		printf("%s0x%08"PRIX64"\n", (verbose > 0) ? "Physical address: " : "", physical_address);
	}

	// FIXME

	return EXIT_SUCCESS;
}

// help info
void usage(FILE * stream, char *self)
{
	fprintf(stream,
		"Usage:\n"
		"  %s -h\n"
		"  %s [-q|-v] [-o bits] [-e bits] [-f bits] [-t page_table_data_file] linear_address(es)\n"
		"Purpose:\n"
		"  Translate given linear (virtual) address to physical using given page table.\n"
		"Options:\n"
		"  -h		help\n"
		"  -o bits	offset size in bits (default: %d)\n"
		"  -e bits	page table entry size in bits (default: %d)\n"
		"  -f bits	limit frame size in bits (default depends on values of -e and -o)\n"
		"  -t fname	file containing page table(s) (default: '%s')\n"
		"  -q		be quiet\n"
		"  -v		more verbosity (default verbosity: %d)\n"
		"Arguments:\n"
		"  linear_address(es) -- linear address(es) to be translated (%d bits)\n",
		self, self,
		DEFAULT_OFFSET_SIZE,
		DEFAULT_PAGE_TABLE_ENTRY_SIZE,
		DEFAULT_PT_FNAME,
		DEFAULT_VERBOSITY,
		LINEAR_ADDRESS_SIZE
		);
}

// parse options
void eval_args(int argc, char *argv[])
{
	int opt;		// option

	opterr = 0;		// don't print error messages on unknown options
	// options processing
	while ((opt = getopt(argc, argv, "hqvo:e:f:t:")) != -1) {
		switch (opt) {
		// -o offset_bits 
		case 'o':
			offset_size = strtol(optarg, NULL, 0);
			if (offset_size > LINEAR_ADDRESS_SIZE || offset_size < 4) {
				fprintf(stderr, "%d: Invalid offset bits.\n",
					offset_size);
				exit(EXIT_FAILURE);
			}
			break;
		// -e page_table_entry_size
		case 'e':
			page_table_entry_size = strtol(optarg, NULL, 0);
			switch (page_table_entry_size) {
				case 32:
				case 64:
					break;
				default:
					fprintf(stderr, "%d: Invalid page table entry size.\n",
						page_table_entry_size);
					exit(EXIT_FAILURE);
			}
			break;
		// -f frame_size_limit
		case 'f':
			frame_size_limit = strtol(optarg, NULL, 0);
			break;
		// -t page_table_filename
		case 't':
			pt_filename = optarg;
			break;
		// quiet
		case 'q':
			verbose = 0;
			break;
		// more verbosity
		case 'v':
			verbose++;
			break;
		// help
		case 'h':
			usage(stdout, argv[0]);
			exit(EXIT_SUCCESS);
			break;
		// unnown option
		default:
			fprintf(stderr, "%c: unknown option.\n", optopt);
			usage(stderr, argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	// page table entry bit holding least significant bit of a frame
	frame_start_bit = offset_size;
	// adjust bits of a frame number
	frame_size = page_table_entry_size - offset_size;
	// limit frame number size if user requested so
	if (frame_size > frame_size_limit)
		frame_size = frame_size_limit;
	return;
}
// EOF
