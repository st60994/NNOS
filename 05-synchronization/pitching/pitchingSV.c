// Operating Systems: sample code  (c) Tomáš Hudec
// Threads
// Synchronization
// System V semaphores
//
// Modified: 2019-12-03
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <pthread.h>

#define SPEED_SLOW	  0	// id of slow
#define P_FAST		 70	// probability of the fast throw
volatile short speed_status = 2;
char *speed_status_text[] = { "SLOW", "FAST", "UNDEFINED" };

#define HIT_SHORT	  0	// id of short
#define P_FAST_LONG	 80	// probability of the long hit for the fast throw
#define P_SLOW_LONG	 40	// probability of the long hit for the slow throw
volatile short hit_status = 2;
char *hit_status_text[] = { "SHORT", "LONG", "UNDEFINED" };

#define P_LONG_CATCHED	  2	// probability of the catch for the long hit
#define P_SHORT_CATCHED	 40	// probability of the catch for the short hit
volatile short catch_status = 2;
char *catch_status_text[] = { "NOT CATCHED", "CATCHED", "UNDEFINED" };


// return 0 or 1 by random based on the given probability percentage
__attribute__ ((always_inline)) static inline
unsigned short probably(unsigned short probability)
{
	return (100.0 * (double)rand() / ((double)RAND_MAX + 1.0)) < probability;
}

// the thread function simulating the pitcher behaviour:
// throwing and trying to catch the ball
void *pitcher(void *arg)
{
	printf("pitcher: Throwing a pitch (the ball) at the batter: %s speed.\n",
		speed_status_text[speed_status = probably(P_FAST)]);

	printf("pitcher: Trying to field (catch) the %s ball that was hit by the batter: %s\n",
		hit_status_text[hit_status],
		catch_status_text[catch_status = probably(HIT_SHORT == hit_status ? P_SHORT_CATCHED : P_LONG_CATCHED)]);

	return NULL;
}

// the thread function simulating the batter behaviour:
// trying to hit the ball and checking the catch status
void *batter(void *arg)
{
	printf("batter:  Batting the %s ball: %s distance\n",
		speed_status_text[speed_status],
		hit_status_text[hit_status = probably(SPEED_SLOW == speed_status ? P_SLOW_LONG : P_FAST_LONG)]);

	printf("batter:  Dropping the bat and running towards the first base.\n");

	printf("batter:  At the 1st base: Checking the ball status: %s.\n", catch_status_text[catch_status]);

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid_batter, tid_pitcher;	// thread ids

	srand(time(NULL) + getpid());	// initialize random number genereator

	// create two threads: batter and pitcher
	if (pthread_create(&tid_batter, NULL, batter, NULL)) {
		perror("pthread_create: batter");
		return EXIT_FAILURE;
	}
	if (pthread_create(&tid_pitcher, NULL, pitcher, NULL)) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}

	// wait for both threads
	if (pthread_join(tid_batter, NULL)) {
		perror("pthread_join: batter");
		return EXIT_FAILURE;
	}
	if (pthread_join(tid_pitcher, NULL)) {
		perror("pthread_join: pitcher");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
