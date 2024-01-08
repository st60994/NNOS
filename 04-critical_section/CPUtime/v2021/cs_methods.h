// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Section Access Control
// header file

// Modified: 2017-11-30, 2017-12-06, 2020-11-25, 2020-12-10

#define CS_METHOD_LOCKED		0
#define CS_METHOD_PETERSON		1
#define CS_METHOD_PETERSON_XCHG		2
#define CS_METHOD_XCHG			3
#define CS_METHOD_TEST_XCHG		4
#define CS_METHOD_MUTEX			5
#define CS_METHOD_SEM_POSIX		6
#define CS_METHOD_SEM_POSIX_NAMED	7
#define CS_METHOD_SEM_SYSV		8
#define CS_METHOD_MQ_POSIX		9
#define CS_METHOD_MQ_SYSV		10

#define CS_METHODS			11
#define CS_METHODS_BUSY_WAIT		5

#include <stdbool.h>			// bool
#include <stdlib.h>			// exit
#include <stdio.h>			// fprintf

bool busy_wait_yields = false;

// variable declarations and function definitions for critical section access


// note: inline is not used unless asked for optimization
#define FORCE_INLINE	__attribute__ ((always_inline)) static inline

// allocate/initialize variables used for the critical section access control
FORCE_INLINE
void cs_init(int method, int threads);

// destroy allocated variables used for the critical section access control
FORCE_INLINE
void cs_destroy(void);

// before entering the critical section
FORCE_INLINE
void cs_enter(int id);

// after leaving the critical section
FORCE_INLINE
void cs_leave(int id);


static int cs_method_used = -1;		// method used, initialized in cs_init()
static bool cs_var_allocated = false;	// successful allocation of variables


// implementation (the funcions are to be inlined, we need them here)


// destroy allocated variables used for the critical section access control
void cs_destroy(void)
{
	if (!cs_var_allocated)		// if not allocated: nothing to do
		return;
	switch (cs_method_used) {
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_PETERSON:
		break;
	case CS_METHOD_PETERSON_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// check the number of threads
FORCE_INLINE 
void Peterson_check(int threads)
{
	if (2 != threads) {
		fprintf(stderr, "Error: Petersons's algorithm is limited to two theads.\n");
		exit(EXIT_FAILURE);
	}
}

// allocate/initialize variables used for the critical section access control
void cs_init(int method, int threads)
{
	cs_method_used = method;
	switch (cs_method_used) {
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_PETERSON:
		Peterson_check(threads);
		break;
	case CS_METHOD_PETERSON_XCHG:
		Peterson_check(threads);
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	default:
		fprintf(stderr, "Error: The method %d is not defined.\n", cs_method_used);
		exit(EXIT_FAILURE);
	}
	cs_var_allocated = true;
}

// before entering the critical section
void cs_enter(int id)
{
	switch (cs_method_used) {
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_PETERSON:
		break;
	case CS_METHOD_PETERSON_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// after leaving the critical section
void cs_leave(int id)
{
	switch (cs_method_used) {
	case CS_METHOD_LOCKED:
		break;
	case CS_METHOD_PETERSON:
		break;
	case CS_METHOD_PETERSON_XCHG:
		break;
	case CS_METHOD_XCHG:
		break;
	case CS_METHOD_TEST_XCHG:
		break;
	case CS_METHOD_MUTEX:
		break;
	case CS_METHOD_SEM_POSIX:
		break;
	case CS_METHOD_SEM_POSIX_NAMED:
		break;
	case CS_METHOD_SEM_SYSV:
		break;
	case CS_METHOD_MQ_POSIX:
		break;
	case CS_METHOD_MQ_SYSV:
		break;
	}
}

// EOF
