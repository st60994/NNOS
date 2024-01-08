// Operating Systems: sample code  (c) Tomáš Hudec
// IPC: shared memory
// System V:	shared data

#define ATTACH_FAILED	((void *) -1)

typedef struct mem_texts {
	int count;
	char texts[0];
} mem_texts_t;

// EOF
