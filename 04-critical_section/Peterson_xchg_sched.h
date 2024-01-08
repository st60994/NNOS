// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// Peterson's algorithm / Petersonův algoritmus
//
// Last modification: 2015-11-25, 2020-12-02
//
// The race on SMP systems is avoided by using xchg instruction
// which synchronizes the CPU-caches on all processors.
// Selhání na systémech SMP je zabráněno použitím instrukce xchg,
// která způsobí aktualizaci cache na všech procesorech.

#include <stdbool.h>
#include <sched.h>		// sched_yield(2)

// Peterson's data: flag for each process/thread and turn
// static variables are always initialized to 0 (C standard)
static volatile	
bool flag[2];
static volatile
int turn;

// Peterson's init
static inline	// note: inline is not used unless asked for optimization
void Peterson_init(void) __attribute__((always_inline));

// Peterson's wait
static inline	// note: inline is not used unless asked for optimization
void Peterson_wait(int self) __attribute__((always_inline));

// Peterson's signal
static inline
void Peterson_post(int self) __attribute__((always_inline));


// Peterson's init
static inline	// note: inline is not used unless asked for optimization
void Peterson_init(void)
{
	flag[0] = false;	// thread 0: access to the critical section NOT needed
	flag[1] = false;	// thread 1: access to the critical section NOT needed
	// turn = 0;		// not needed, it's always set before reading
}
 
// Peterson's wait
static inline	// note: inline is not used unless asked for optimization
void Peterson_wait(int self)
{
#ifdef __atomic_store_n
	__atomic_store_n(&flag[self], true, __ATOMIC_RELAXED);	// flag[self] = true;   // require access to the critical section
	__atomic_exchange_n(&turn, !self, __ATOMIC_ACQ_REL);	// turn = !self;        // the other thread takes precedence
	// busy wait while the other thread requires access and has precedence
	while (__atomic_load_n(&flag[!self], __ATOMIC_ACQUIRE) && __atomic_load_n(&turn, __ATOMIC_RELAXED) == !self)
		sched_yield();	// give up the CPU while busy waiting
#else
	// the xchg instruction forces CPU-cache flush
	asm volatile (
		"xchg %0,%b2\n\t"	// flag[self] = true;	// require access to the critical section
		//"mfence\n\t"		// prevent CPU reordering
		"xchg %1,%3"		// turn = !self;	// the other thread takes precedence
		: "=m" (flag[self]),
		  "=m" (turn)		// output: flag[self] and turn
		: "r" (true),
		  "r" (!self)		// input: true and !self (both stored to registers)
		: "memory"		// memory is changed, tells gcc NOT to keep variables in registers
	);
	// busy wait while the other thread requires access and has precedence
	while (flag[!self] && turn != self)
		sched_yield();	// give up the CPU
#endif
}

// Peterson's signal
static inline	// note: inline is not used unless asked for optimization
void Peterson_post(int self)
{
#ifdef __atomic_store_n
	__atomic_store_n(&flag[self], false, __ATOMIC_RELEASE);	// leave the critical section
#else
	flag[self] = false;		// leave the critical section, simple assignment is sufficient
	/*
	// in case of required cache flush:
	asm volatile (
		//"mfence\n\t"		// prevent CPU reordering
		"xchg %0,%b1"		// flag[self] = false;
		: "+m" (flag[self])	// output: flag[self]
		: "r" (false)		// input: false
		: "memory"		// memory is changed, tells gcc NOT to keep variables in registers
	);
	*/
#endif
}

// EOF
