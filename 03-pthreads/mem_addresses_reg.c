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
#include <inttypes.h>

// text macros to print names of variables
// textová makra pro výpis jmen proměnnách
#define __TEXT(X)	#X
#define _TEXT(X)	__TEXT(X)

int global = 5;	// global static variable / globální statická proměnná

uint16_t ss;	// ss selector
#if _WIN64 || __amd64__ || __x86_64__
// 64-bit
uint64_t rsp, rbp;
#else
// 32-bit
uint32_t esp, ebp;
#endif

// subroutine called from another subroutine / podprogram volaný z jiného podprogramu
void subsubroutine(int arg1, int arg2)
{
	int subsub = 3;	// local variable (allocated on the stack)

	printf("Thread %d, value and address of the subroutine parameter:             "
		"%-9s %-2d %14p\n", arg1, _TEXT(arg2), arg2, &arg2);
	printf("Thread %d, value and address of the subroutine parameter:             "
		"%-9s %-2d %14p\n", arg1, _TEXT(arg1), arg1, &arg1);
	
	printf("Thread %d, value and address of the local variable in a subroutine:   "
		"%-9s %-2d %14p\n", arg1, _TEXT(subsub), subsub, &subsub);

	return;
}


// subroutine / podprogram
void *subroutine(void *arg)
{
	int sub = *(int *)arg;			// local variable on the stack
	int *subheap = malloc(sizeof(int));	// run-time allocated variable in the global space

	*subheap = 6;
	printf("Thread %d, value and address of the dynamic variable in a subroutine: "
		"%-9s %-2d %14p\n", sub, _TEXT(*subheap), *subheap, subheap);
	printf("Thread %d, value and address of the local variable in a subroutine:   "
		"%-9s %-2d %14p\n", sub, _TEXT(sub), sub, &sub);
	
	subsubroutine(sub, 0);
	return NULL;
}

int main(int argc, char *argv[])
{
	int main_id = 1;			// id of main thread
	pthread_t a_thread;	// thread id
	int rc = 0;
	int thread_id = 2;			// id of created thread
	int *heap = malloc(sizeof(int));	// run-time allocated variable in the global space
	int main_id2 = 2;			// id of main thread

	*heap = 4;
	printf("Thread %d, value and address of the global static variable:           "
		"%-9s %-2d %14p\n", main_id, _TEXT(global), global, &global);
	printf("Thread %d, value and address of the global dynamic variable:          "
		"%-9s %-2d %14p\n", main_id, _TEXT(*heap), *heap, heap);
	printf("Thread %d, value and address of the local variable in main():         "
		"%-9s %-2d %14p\n", main_id, _TEXT(main_id), main_id, &main_id);
	printf("Thread %d, value and address of the local variable in main():         "
		"%-9s %-2d %14p\n", main_id2, _TEXT(main_id2), main_id2, &main_id2);
	ss = 0;
#if _WIN64 || __amd64__ || __x86_64__
	rsp = rbp = 0;	// 64-bit
	asm volatile (
		"xorl	%%eax,%%eax	\n"
		"movw	%%ss,%%ax	\n"
		"movw	%%ax,%0		\n"
		"movq	%%rsp,%1	\n"
		"movq	%%rbp,%2	\n"
		: "=m"	(ss),	"=m"	(rsp),	"=m"	(rbp)	// output
		:						// input
		: "memory", "%eax"	// clobbered register(s)
	);
	printf("SS:RSP,RBP =  %x:%lx,%lx\n\tSS+RSP = %lx\n\tSS+RBP = %lx\n", ss, rsp, rbp, ss+rsp, ss+rbp);
#else
	esp = ebp = 0;	// 32-bit
	asm volatile (
		"xorl	%%eax,%%eax	\n"
		"movw	%%ss,%%ax	\n"
		"movw	%%ax,%0		\n"
		"movl	%%esp,%1	\n"
		"movl	%%ebp,%2	\n"
		: "=m"	(ss),	"=m"	(esp),	"=m"	(ebp)	// output
		:						// input
		: "memory", "%eax"	// clobbered register(s)
	);
	printf("SS:ESP,EBP =  %x:%x,%x\n\tSS+ESP = %x\n\tSS+EBP = %x\n", ss, esp, ebp, ss+esp, ss+ebp);
#endif
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
