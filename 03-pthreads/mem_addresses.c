// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// show logical (virtual) addresses of different variables
// (static global, dynamic, local) and function arguments in different threads
// zobrazení logických (virtuálních) adres různých proměnných
// (staticých globálních, dynamických, lokálních) a argumentů funkcí v různých vlánech
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <inttypes.h>		// to print pointers (adresses of variables)

// text macros to print names of variables
// textová makra pro výpis jmen proměnných
#define __TEXT(X)	#X
#define _TEXT(X)	__TEXT(X)

int global = 5;	// global static variable / globální statická proměnná

// subroutine called from another subroutine / podprogram volaný z jiného podprogramu
void subsubroutine(int arg1, int arg2)
{
	int subsub1 = 3;	// local variable (allocated on the stack)
	int subsub2 = 4;	// local variable (allocated on the stack)

	printf("Thread %d,     the subroutine parameter:           "
		"%-9s %-2d %#014" PRIxPTR "\n", arg1, _TEXT(arg2), arg2, (uintptr_t)&arg2);
	printf("Thread %d,     the subroutine parameter:           "
		"%-9s %-2d %#014" PRIxPTR "\n", arg1, _TEXT(arg1), arg1, (uintptr_t)&arg1);
	
	printf("Thread %d,     the local variable in a subroutine: "
		"%-9s %-2d %#014" PRIxPTR "\n", arg1, _TEXT(subsub1), subsub1, (uintptr_t)&subsub1);
	printf("Thread %d,     the local variable in a subroutine: "
		"%-9s %-2d %#014" PRIxPTR "\n", arg1, _TEXT(subsub2), subsub2, (uintptr_t)&subsub2);

	return;
}


// subroutine / podprogram
void *subroutine(void *arg)
{
	int sub1 = *(int *)arg;			// local variable on the stack
	int sub2 = 8;				// local variable on the stack
	int *subheap1 = malloc(sizeof(int));	// run-time allocated variable in the global space
	int *subheap2 = malloc(sizeof(int));	// run-time allocated variable in the global space

	*subheap1 = 6;
	*subheap2 = 9;
	printf("Thread %d, the dynamic variable in a subroutine:   "
		"%-9s %-2d %#014" PRIxPTR "\n", sub1, _TEXT(*subheap1), *subheap1, (uintptr_t)subheap1);
	printf("Thread %d, the dynamic variable in a subroutine:   "
		"%-9s %-2d %#014" PRIxPTR "\n", sub1, _TEXT(*subheap2), *subheap2, (uintptr_t)subheap2);
	printf("Thread %d,   the local variable in a subroutine:   "
		"%-9s %-2d %#014" PRIxPTR "\n", sub1, _TEXT(sub1), sub1, (uintptr_t)&sub1);
	printf("Thread %d,   the local variable in a subroutine:   "
		"%-9s %-2d %#014" PRIxPTR "\n", sub1, _TEXT(sub2), sub2, (uintptr_t)&sub2);
	
	subsubroutine(sub1, 0);
	return NULL;
}

int main(int argc, char *argv[])
{
	int rc = 0;
	int main_id = 1;			// id of main thread
	int thread_id = 2;			// id of created thread
	int *heap1 = malloc(sizeof(int));	// run-time allocated variable in the global space
	int *heap2 = malloc(sizeof(int));	// run-time allocated variable in the global space
	pthread_t a_thread;	// thread id

	*heap1 = 4;
	*heap2 = 7;
	// printf("Values and addresses of various variables:\n");
	printf("Thread %d, the global static variable:             "
		"%-9s %-2d %#014" PRIxPTR "\n", main_id, _TEXT(global), global, (uintptr_t)&global);
	printf("Thread %d, the global dynamic variable:            "
		"%-9s %-2d %#014" PRIxPTR "\n", main_id, _TEXT(*heap1), *heap1, (uintptr_t)heap1);
	printf("Thread %d, the global dynamic variable:            "
		"%-9s %-2d %#014" PRIxPTR "\n", main_id, _TEXT(*heap2), *heap2, (uintptr_t)heap2);
	printf("Thread %d, the local variable in main():           "
		"%-9s %-2d %#014" PRIxPTR "\n", main_id, _TEXT(rc), rc, (uintptr_t)&rc);
	printf("Thread %d, the local variable in main():           "
		"%-9s %-2d %#014" PRIxPTR "\n", main_id, _TEXT(main_id), main_id, (uintptr_t)&main_id);
	subroutine(&main_id);

	// create another thread
	rc = pthread_create(&a_thread, NULL, subroutine, &thread_id);
	if (rc != 0) {
		perror("pthread_create failed");
		exit(EXIT_FAILURE);
	}
	pthread_join(a_thread, NULL);	// wait for the thread finish
	
	return EXIT_SUCCESS;
}

// memory of a process
// 
// ┏━━━━━━━┓ max address
// ┃ stack ┃ main thread stack
// ┠───↓───┨
// ┃       ┃
// ┠───────┨
// ┃ stack ┃ another thread stack
// ┠───↓───┨
// ┃       ┃
// ╹---⚡---╹
// ╻---⚡---╻
// ┃       ┃
// ┠───↑───┨
// ┃ heap  ┃ dynamic data
// ┠───────┨
// ┃ data  ┃ static data
// ┠───────┨
// ┃ code  ┃
// ┗━━━━━━━┛ min address (0)
