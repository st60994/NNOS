/*
 * Synchronizace vláken
 * Zadání modifikováno: 2011-12-21
 *
 * Problém:
 *
 * Máme sadu výpočtů (proměnlivý počet, pořadí dané, délka výpočtů různá)
 * a omezený počet výpočetních vláken pro tyto výpočty.
 *
 * Jedno „sběrné“ vlákno sbírá výsledky a tiskne je postupně (dle pořadí)
 * na výstup.
 *
 * Hlavní vlákno vytvoří „sběrné“ vlákno a pak postupně alokuje a spouští
 * pracovní vlákna pro jednotlivé výpočty. Je-li je počet výpočtů větší
 * než počet pracovních vláken, některé výpočty lze zahájit až po dokončení
 * jiných.
 *
 * Řešte synchronizaci:
 * Hlavní vlákno řeší,
 *  • zda je volné výpočetní vlákno pro výpočet,
 *  • je-li vlákno volné, alokuje je a spustí další výpočet.
 * Sběrné vlákno řeší,
 *  • zda byl výpočet dokončen
 * Výpočetní (pracovní) vlákno
 *  • signalizuje, že výpočet byl dokončen
 *  • signalizuje volné vlákno (takže je hlavní vlákno může alokovat).
 *
 *******************************************************************************

    HLAVNÍ            → → → → → → → → → → → → → → → →  SBĚRNÉ
       ↓             ↑                                    ↓
    [vytvoř sběrné] →                                  <pro každý výpočet w>  ←
       ↓                                                  ↓                    ↑
    [vytvoř n pracovních] → →             → [čekej] ← −<ukončen výpočet w?>    ↑
       ↓                     ↓           ↑     ↓   (ne)   +(ano)               ↑
 →  <pro každý výpočet w>  PRACOVNÍ      ↑      → → → → → ↓                    ↑
↑      ↓                   <cyklus>      ↑                ↓                    ↑
↑   [send w] → <mq> → → →  [receive w]    ← ←          [vypiš výsledek w]      ↑
↑      ↓                   [výpočet w]       ↑            ↓                    ↑
 ← −<poslední?>            [signál výpočet] →          <poslední?>− → → → → → →
 (ne)  +(ano)              <další iterace>                +(ano)  (ne)
       ↓                                                  -
    [join sběrné]  ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← (konec)
       -
    [ukonči n pracovních]
       -
    (konec)

 *******************************************************************************
 */

#define ME		"sync_working_threads"
#define VERSION		"2.1 (2011)"
// text
#define __TEXT(X)	#X
#define _TEXT(X)	__TEXT(X)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <locale.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>


#define MAX_WORK	1000	// max. počet všech výpočtů
#define DEF_WORK	33	// výchozí počet výpočtů

#define MAX_WAIT_S	10.0	// max. simulovaná doba (provádění výpočtu)
#define DEF_WAIT_S	5.0	// výchozí hodnota pro max. náhodnou dobu čekání

// stavy
#define	WORK_STATE_IDLE	'_'	// nezahájeno
#define	WORK_STATE_BUSY	'w'	// pracuje
#define	WORK_STATE_DONE	'F'	// dokončeno

// working info
struct work_t {
	double	result;		// výsledek výpočtu
	char	state;		// stav: _ = nezahájeno, w = pracuje, F = dokončeno
	// synchronizace dokončení výpočtu
	sem_t	done;		// semafor informující, že výpočet byl dokončen
};

struct work_t *work;		// struktura pro pracovní výpočty
int num_work = DEF_WORK;	// počet pracovních výpočtů
double wait_s = DEF_WAIT_S;	// max. simulovaná (náhodná) doba výpočtu

// pro korektní zobrazení stavů výpočtů
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_THREADS	100	// max. počet výpočetních vláken (nikoli vláken celkem)
#define DEF_THREADS	10	// výchozí počet výpočetních vláken

struct threads_t {
	pthread_t	id;	// id vlákna
	int		arg;	// argument vláknové funkce
};

struct threads_t *threads;	// struktura pro výpočetní vlákna
int num_threads = DEF_THREADS;	// max. počet výpočetních vláken

#define	MQ_NAME		"/sync_working_threads_mq_sem:st" _doplnit_NetID_	// jméno fronty
#define MQ_LEN		10	// max. počet zpráv ve frontě

union message_t {
	char		msg[1];	// zpráva se očekává typu pole znaků
	int		w;	// potřebujeme předávat int
};

mqd_t work_queue;		// deskriptor fronty výpočtů pro vlákna
struct mq_attr mqattr;		// atributy fronty

sem_t threads_used;		// aktuální počet používaných výpočetních vláken
int max_threads_used = 0;	// tolik výpočetních vláken běželo současně nejvíc

double	time_start;		// čas zahájení
double	time_total = 0;		// čas trvání celkem (reálný čas běhu)
double	time_threads_sum = 0;	// součet časů běhu jednotlivých výpočetních vláken

int verbose = 0;		// nějaké hlášky navíc?

// čas jako reálné číslo
double get_time(void)
{
	struct timeval tv;
	double d;

	gettimeofday(&tv, NULL);

	// usec je v mikrosekundách
	d = ((double) tv.tv_usec) / 1000000.0 + (unsigned long) tv.tv_sec;

	return d;
}

// skutečné provedení výpočtu
void do_work(int w)
{
	// vrací pseudo-náhodné číslo od 1 do wait_s * 1000 (milisekund)
	int r = 1 + (int) (wait_s * 1000.0 * (rand() / (RAND_MAX + 1.0)));
	double time_start_computing = get_time();

	usleep(1000 * r);	// simulace činnosti

	// výsledkem je celková doba běhu této funkce
	work[w].result = get_time() - time_start_computing;

	return;
}

void print_results(int w)
{
	if (verbose > 0)
		printf("%2d. výpočet dokončen: %5.3f s\n", w+1, work[w].result);
}

void print_states(void)
{
	int w;

	// vypsání stavu všech výpočtů
	pthread_mutex_lock(&print_mutex);
	fprintf(stderr, "Stavy: ");
	for (w = 0; w < num_work; ++w)
		fprintf(stderr, "%c", work[w].state);
	fprintf(stderr, "\r");
	fflush(stderr);
	pthread_mutex_unlock(&print_mutex);
}

void *working_thread(void *arg)
{
	int thread_idx = *(int *)arg;
	union message_t msg;
	int w;
	int i;

	while (1) {
		// zjištění, který výpočet má probíhat
		if ((mq_receive(work_queue, msg.msg, sizeof(union message_t), NULL)) == -1) {
			perror("mq_receive");
			exit(EXIT_FAILURE);
		}
		w = msg.w;

		// atomické zvýšení počtu současně pracujících vláken
		sem_post(&threads_used);
		// zjisti počet použitých vláken
		sem_getvalue(&threads_used, &i);
		// maximálně lze mít num_threads vláken, použitých je i
		if (i > max_threads_used)
			max_threads_used = i;

		// zahájení výpočtu
		if (verbose > 1)
			printf("    Vlákno %2d: výpočet %2d zahájen.\n", thread_idx, w+1);
		work[w].state = WORK_STATE_BUSY;
		if (verbose < 1)	// pokud nevypisujeme výsledky, zobrazíme stavy
			print_states();
		do_work(w);
		work[w].state = WORK_STATE_DONE;
		if (verbose < 1)	// pokud nevypisujeme výsledky, zobrazíme stavy
			print_states();
		if (verbose > 1)
			printf("    Vlákno %2d: výpočet %2d dokončen.\n", thread_idx, w+1);

		// signalizace dokončení výpočtu
		// done

		// atomické snížení počtu současně pracujících vláken
		sem_wait(&threads_used);
	}
	// return NULL;
}

void *results_thread(void *arg)
{
	int w;

	time_threads_sum = 0;
	for (w = 0; w < num_work; w++) {
		// čekání na dokončení práce vlákna pro výpočet w
		// done

		// uvolníme prostředky pro tento semafor
		// done

		// tisk výsledků
		print_results(w);
		time_threads_sum += work[w].result;
	}

	return NULL;
}

void catch_exit(int sig);

int init_work()
{
	int w;
	struct sigaction sa;

	// alokace pole pro výsledky výpočtů, automaticky nulováno
	work = calloc(num_work, sizeof(struct work_t));
	if (work == NULL) {
		perror("work calloc");
		exit(EXIT_FAILURE);
	}

	// inicializace semaforů dokončení výpočtu
	for (w = 0; w < num_work; w++) {
		// inicializace semaforu pro synchronizaci
		// done

		// inicializace stavu
		work[w].state = WORK_STATE_IDLE;
	}

	// alokace a inicializace pole vláken na nuly
	threads = calloc(num_threads, sizeof(struct threads_t));
	if (threads == NULL) {
		perror("threads calloc");
		exit(EXIT_FAILURE);
	}

	// inicializace pomocného semaforu použitých vláken
	if (sem_init(&threads_used, 0, 0)) {
		perror("sem_init(threads_available)");
		exit(EXIT_FAILURE);
	}

	// inicializace fronty zpráv
	mq_unlink(MQ_NAME);	// bez kontroly chyb
	// atributy fronty
	mqattr.mq_flags = 0;
	mqattr.mq_maxmsg = MQ_LEN;
	mqattr.mq_msgsize = sizeof(union message_t);
	mqattr.mq_curmsgs = 0;
	// alokace nové fronty
	work_queue = mq_open(MQ_NAME, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR, &mqattr);
	if (work_queue == (mqd_t) -1) {
		perror("mq_open");
		exit(EXIT_FAILURE);
	}

	// handle clean exit
	sa.sa_handler = catch_exit;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	return 0;
}

int release_resources()
{
	// semafory pro synchronizaci dokončení jsou uvolněny ihned po dokončení výpočtu
	// uvolnění pole výpočtů
	free(work);

	// uvolnění pole vláken
	free(threads);

	// uvolnění pomocného semaforu použitých vláken
	if (sem_destroy(&threads_used) == -1)
		perror("sem_init(threads_available)");

	// uzavření fronty zpráv
	if (mq_close(work_queue) == (mqd_t) -1)
		perror("mq_close");

	// odstranění fronty zpráv
	if (mq_unlink(MQ_NAME) == -1)
		perror("mq_unlink");

	return 0;
}

void catch_exit(int sig) {
	release_resources();
	exit(EXIT_FAILURE);
}

// vytváření vláken
void do_all()
{
	pthread_t	results_thread_id;
	int		w;
	int		rc;
	int		thread_index;
	union message_t	msg;

	// inicializace
	init_work();
	
	if (verbose > 1)
		printf("Spouští se vlákno pro sběr výsledků.\n");
		
	// vlákno na zpracování výsledků (jejich tisk)
	pthread_create(&results_thread_id, NULL, results_thread, NULL);

	if (verbose)
		printf("Výpočtů celkem: %d\n"
		       "Počet vláken:   %d\n", num_work, num_threads);
		
	// vytvoření všech výpočetních vláken
	for (thread_index = 0; thread_index < (num_threads > num_work ? num_work : num_threads); thread_index++) {
		threads[thread_index].arg = thread_index;
		rc = pthread_create(&threads[thread_index].id, NULL,
			working_thread, &threads[thread_index].arg);
		if (rc) {
			fprintf(stderr, "\nCannot create thread %d: %s\n", thread_index, strerror(rc));
			exit(EXIT_FAILURE);
		}
	}

	// měříme čas
	time_start = get_time();

	// spouštění výpočtů
	for (w = 0; w < num_work; w++) {
		// dej do fronty další požadavek na výpočet
		msg.w = w;
		mq_send(work_queue, msg.msg, sizeof(union message_t), 0);
	}

	// čekáme na dokončení všech výpočtů
	rc = pthread_join(results_thread_id, NULL);

	time_total = get_time() - time_start;	// kolik času to trvalo celkem

	// ukončíme všechna výpočetní vlákna
	for (thread_index = 0; thread_index < (num_threads > num_work ? num_work : num_threads); thread_index++) {
		rc = pthread_cancel(threads[thread_index].id);
		if (rc) {
			fprintf(stderr, "\nCannot cancel thread %d: %s\n", thread_index, strerror(rc));
			exit(EXIT_FAILURE);
		}
	}
	
	if (verbose < 1)	// při zobrazování stavů
		fprintf(stderr, "\n");

	fprintf(stderr, "Maximum současně běžících výpočetních vláken: %d\n",
		max_threads_used);

	printf("Celkový čas běhu (reálný): %6.3f\n"
	       "Celkový čas běhu vláken:   %6.3f\n"
	       "Úspora času:               %2.1f %%\n",
	       time_total, time_threads_sum,
	       (time_threads_sum - time_total) * 100 / time_threads_sum);

	// uvolnění prostředků
	release_resources();

	// hotovo
	return;
}

// použití
void usage(FILE *f)
{
	fprintf(f,
	"Použití:\n"
	"  %s [-hVvq] [-t počet_vláken] [-n počet_výpočtů] [-w sekundy]\n"
	"Volby:\n"
	"  -h                Tato nápověaa.\n"
	"  -V                Zobraz verzi a ukonči se.\n"
	"  -v                Více hlášek.\n"
	"  -q                Méně hlášek.\n"
	"  -t počet_vláken   Použij tento počet výpočetních vláken současně (" _TEXT(DEF_THREADS) "). Maximum je " _TEXT(MAX_THREADS) ".\n"
	"  -n počet_výpočtů  Simuluj tento počet výpočtů (" _TEXT(DEF_WORK) "). Maximum je " _TEXT(MAX_WORK) ").\n"
	"  -w sekundy        Simulace výpočtu trvá maximálně daný počet sekund (" _TEXT(DEF_WAIT_S) "). Maximum je " _TEXT(MAX_WAIT_S) ".\n"
	"",
	ME);
}

// zpracování voleb
void parse_options(int argc, char *argv[])
{
	int opt;		// options

	while ((opt = getopt(argc, argv, "hVvqt:n:w:")) != -1) {
		switch (opt) {
			case 'h':
				usage(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
				printf("%s version %s\n", ME, VERSION);
				exit(EXIT_SUCCESS);
			case 'v':
				if (verbose < 100)
					verbose++;
				break;
			case 'q':
				if (verbose > 0)
					verbose--;
				break;
			case 'n':
				num_work = atoi(optarg);
				if (num_work > MAX_WORK)
					num_work = MAX_WORK;
				if (num_work < 1)
					num_work = 1;
				break;
			case 't':
				num_threads = atoi(optarg);
				if (num_threads > MAX_THREADS)
					num_threads = MAX_THREADS;
				if (num_threads < 1)
					num_threads = 1;
				break;
			case 'w':
				wait_s = atof(optarg);
				if (wait_s > MAX_WAIT_S)
					wait_s = MAX_WAIT_S;
				if (wait_s < 0)
					wait_s = 0;
				break;
			default:
				usage(stderr);
				exit(EXIT_FAILURE);
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	setlocale (LC_ALL, "");
	srand(time(NULL) + getpid());

	// zpracuj přepínače
	parse_options(argc, argv);

	do_all();

	return EXIT_SUCCESS;
}
