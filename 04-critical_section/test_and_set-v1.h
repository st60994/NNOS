// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// HW method: test-and-set (xchg)
//
// Last modification: 2015-11-25
//
// use:
// volatile int lock = 0;
//
// ret = test_and_set(&lock);

// atomic store of 1 into *lock, returns previous value of *lock
static inline	// note: inline is not used unless asked for optimization
int test_and_set(volatile int *lock) __attribute__((always_inline));


// atomic store of 1 into *lock, returns previous value of *lock
static inline	// note: inline is not used unless asked for optimization
int test_and_set(volatile int *lock)
{
	register int ret = 1;			// value to be stored at *lock

	asm volatile (
		"xchgl %0, %1"
		: "=r" (ret)			// output is previous value of *lock, stored to ret
		: "m" (*lock), "0" (ret)	// input is *lock and ret
		// "0" refers to the same constraint as %0 ("r", i.e. register, in this case)
		: "memory"			// memory is changed, tells gcc NOT to keep variables in registers
	);

	return ret;
}
