/*
 * thread init for threaded traceroute
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "threaded.h"
#include "options.h"
#include "ip_stuff.h"
#include "traceroute.h"


struct work_t *work;		// struktura pro pracovní výpočty
int num_work;			// počet pracovních výpočtů

struct threads_t *threads;	// struktura pro výpočetní vlákna
int num_threads;		// max. počet výpočetních vláken

mqd_t work_queue;		// deskriptor fronty výpočtů pro vlákna
struct mq_attr mqattr;		// atributy fronty

sem_t threads_used;		// aktuální počet používaných výpočetních vláken
int max_threads_used = 0;	// tolik výpočetních vláken běželo současně nejvíc

double	time_start;		// čas zahájení
double	time_total = 0;		// čas trvání celkem (reálný čas běhu)
double	time_threads_sum = 0;	// součet časů běhu jednotlivých výpočetních vláken


int init_work()
{
	int w;
	struct sigaction sa;

	// nastav podle voleb
	num_work = opts.max_ttl * opts.probes;
	num_threads = opts.max_threads;

/* zbytek doplnit */
