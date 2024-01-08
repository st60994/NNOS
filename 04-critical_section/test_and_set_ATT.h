// Operating Systems: sample code  (c) Tomáš Hudec
// Critical Sections
// HW method: test-and-set (xchg)
//
// Modified: 2015-11-25, 2017-12-06
//
// use:
// volatile int lock = 0;
//
// ret = test_and_set(&lock);

// atomic store of 1 into *lock, returns previous value of *lock
__attribute__ ((always_inline)) static inline
int test_and_set(volatile int *lock);


// atomic store of 1 into *lock, returns previous value of *lock
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
