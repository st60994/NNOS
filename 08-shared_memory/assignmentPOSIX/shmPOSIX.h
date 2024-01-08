// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// POSIX: shared data

#define SHM_NAME	"/shmOStask7-login"	// name of the shared memory object

typedef struct mem_texts {
	int count;
	char texts[0];
} mem_texts_t;

// EOF
