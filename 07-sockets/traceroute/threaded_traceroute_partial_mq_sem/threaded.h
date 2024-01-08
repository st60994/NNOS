/*
 * include for threaded traceroute
 */

#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

// working info
struct work_t {
	double		result;	// čas běhu
	// synchronizace dokončení výpočtu
	sem_t		done;	// semafor informující, že výpočet byl dokončen
};

struct threads_t {
	pthread_t	id;	// id vlákna
	int		arg;	// argument vláknové funkce
};

#define	MQ_NAME		"/threaded_traceroute_mq_sem:st" _doplnit_NetID_	// jméno fronty
#define MQ_LEN		10	// max. počet zpráv ve frontě

union message_t {
	char		msg[1];	// zpráva se očekává typu pole znaků
	int		w;	// potřebujeme předávat int
};

int init_work();
int release_resources();
void catch_exit(int sig);
void run_threads();

