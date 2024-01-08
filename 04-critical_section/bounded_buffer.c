/*
 * bounded_buffer.c
 * Code for Producer/Consumer problem using mutex and condition variables
 * To compile me for Unix, type:
 *   gcc -D_REENTRANT -lpthread -o bounded_buffer bounded_buffer.c 
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define BSIZE 4
#define NUMITEMS 30

typedef struct {
	char buf[BSIZE];
	int occupied;
	int nextin, nextout;
	pthread_mutex_t mutex;
	pthread_cond_t more;
	pthread_cond_t less;
} buffer_t;

buffer_t buffer;

void *producer(void *);
void *consumer(void *);


#define NUM_THREADS 2
pthread_t tid[NUM_THREADS];	/* array of thread IDs */

int main(int argc, char *argv[])
{
	int i;

	pthread_cond_init(&(buffer.more), NULL);
	pthread_cond_init(&(buffer.less), NULL);
	pthread_mutex_init(&(buffer.mutex), NULL);

	memset(buffer.buf, '_', sizeof(buffer.buf));
	buffer.occupied = 0;
	buffer.nextin = 0;
	buffer.nextout = 0;

	pthread_create(&tid[1], NULL, consumer, NULL);
	pthread_create(&tid[0], NULL, producer, NULL);
	for (i = 0; i < NUM_THREADS; i++)
		pthread_join(tid[i], NULL);

	printf("\nmain() reporting that all %d threads have terminated\n",
	       i);

	return 0;
}	/* main */


/* ceka do zmeny casu na novou sekundu -- nejvyse jednu sekundu */
static void wait_thread(void)
{
    time_t start_time = time(NULL);
 
    while (time(NULL) == start_time)
    {
        /* do nothing except chew CPU slices for up to one second */
    }
}


void *producer(void *parm)
{
	char item[NUMITEMS] = "IT'S A SMALL WORLD, AFTER ALL.";
	int i;

	wait_thread();		/* cekani na preklopeni casu na novou sekundu */
	printf("Producer started.\n");

	for (i = 0; i < NUMITEMS; i++) {	/* produce an item, one character from item[] */

		if (item[i] == '\0')
			break;	/* Quit if at end of string. */


		if (buffer.occupied >= BSIZE)
			printf("Producer should wait.\n");

		// printf("producer executing.\n");

		buffer.buf[buffer.nextin++] = item[i];
		buffer.nextin %= BSIZE;
		buffer.occupied++;

		/* now: either buffer.occupied < BSIZE and buffer.nextin is the index
		   of the next empty slot in the buffer, or
		   buffer.occupied == BSIZE and buffer.nextin is the index of the
		   next (occupied) slot that will be emptied by a consumer
		   (such as buffer.nextin == buffer.nextout) */

	}
	printf("Producer exiting.\n");
	pthread_exit(0);
}

void *consumer(void *parm)
{
	char item;
	int i;

	wait_thread();		/* cekani na preklopeni casu na novou sekundu */
	printf("Consumer started.\n");

	for (i = 0; i < NUMITEMS; i++) {


		if (buffer.occupied <= 0)
			printf("Consumer should wait.\n");

		// printf("Consumer executing.\n");

		item = buffer.buf[buffer.nextout++];
		printf(" Consumed item: \"%c\"\n", item);
		buffer.nextout %= BSIZE;
		buffer.occupied--;

		/* now: either buffer.occupied > 0 and buffer.nextout is the index
		   of the next occupied slot in the buffer, or
		   buffer.occupied == 0 and buffer.nextout is the index of the next
		   (empty) slot that will be filled by a producer (such as
		   buffer.nextout == buffer.nextin)
		*/

	}
	printf("Consumer exiting.\n");
	pthread_exit(0);
}

