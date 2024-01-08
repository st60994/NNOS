/*
 * Synchronizace vláken
 * Zadání modifikováno: 2011-12-19
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
 *  • zda daný výpočet byl již zahájen (pokud ne, čeká na zahájení),
 *  • byl-li výpočet zahájen (spuštěním výpočetního vlákna), čeká na výsledek,
 *  • signalizuje volné vlákno (takže je hlavní vlákno může alokovat).
 * Výpočetní (pracovní) vlákno
 *  • signalizuje, že výpočet byl zahájen.
 *
 *******************************************************************************

    HLAVNÍ            → → → → → → → → → → → → → → → →  SBĚRNÉ
       ↓             ↑                                    ↓
    [vytvoř sběrné] →                                  <pro každý výpočet w>  ←
       ↓                                                  ↓                    ↑
 →  <pro každý výpočet w>                 → [čekej] ← −<zahájen výpočet w?>    ↑
↑      ↓                                 ↑     ↓   (ne)   +(ano)               ↑
↑   <volné vlákno?>− → [čekej] ← ← ← ← ←   ←    → → → → → ↓                    ↑
↑      +(ano)      (ne)   ↓              ↑  ↑             ↓                    ↑
↑      ↓ ← ← ← ← ← ← ← ← ←               ↑  ↑   → → →  [join výpočetní w]      ↑
↑      ↓                                 ↑  ↑  ↑          ↓                    ↑
↑   [spusť pracovní] → PRACOVNÍ          ↑   ←   ← ← ← [signál volné]          ↑
↑      ↓               [signál výpočet] →      ↑          ↓                    ↑
 ← −<poslední?>        [výpočet]               ↑       <poslední?>− → → → → → →
 (ne)  +(ano)          (konec)  → → → → → → → →           +(ano)  (ne)
       ↓                                                  -
    [join sběrné]  ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← ← (konec)
       -
    (konec)

 *******************************************************************************
 */

#define ME		"sync_working_threads"
#define VERSION		"2.0 (2011)"
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
	int	thread_index;	// index do pole vláken
	double	result;		// výsledek výpočtu
	char	state;		// stav: _ = nezahájeno, w = pracuje, F = dokončeno
	// synchronizace zahájení výpočtu
	sem_t	thread_running;	// semafor informující, že výpočet byl zahájen
	// a proměnné jsou platné (zejména proměnná id vlákna použitá v pthread_join)
};

struct work_t *work;		// struktura pro pracovní výpočty
int num_work = DEF_WORK;	// počet pracovních výpočtů
double wait_s = DEF_WAIT_S;	// max. simulovaná (náhodná) doba výpočtu

// pro korektní zobrazení stavů výpočtů
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_THREADS		100	// max. počet výpočetních vláken (nikoli vláken celkem)
#define DEF_THREADS		10	// výchozí počet výpočetních vláken

#define THREAD_AVAILABLE	0	// musí být 0, inicializace na to spoléhá
#define THREAD_ALLOCATED	1
struct threads_t {
	pthread_t	id;	// id vlákna
	int		state;	// stav vlákna: alokováno / volné
	int		arg;	// argument vláknové funkce
	int		w;	// index do pole výsledků
};

struct threads_t *threads;	// struktura pro výpočetní vlákna
int num_threads = DEF_THREADS;	// max. počet výpočetních vláken
sem_t threads_available;	// aktuální počet volných výpočetních vláken

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


// najde volné vlákno a vráti index do pole vláken
// není-li vlákno volné, čeká na jeho uvolnění
int get_available_thread()
{
	int i;

	// synchronizace na dostupnost vláken
	// threads_available

	// zjisti počet volných vláken
	// threads_available -> i

	// maximálně lze mít num_threads vláken, použitých je num_threads - i
	if (num_threads - i > max_threads_used)
		max_threads_used = num_threads - i;

	// najdeme index, na kterém je volné vlákno
	for (i = 0; i < num_threads; i++) {
		if (threads[i].state == THREAD_AVAILABLE) {
			// alokujeme první volné vlákno
			threads[i].state = THREAD_ALLOCATED;
			// ukončíme procházení vláken
			break;
		}
	}
	// i je nyní index s nově alokovaným vláknem
	// assert
	if (i == num_threads) {	// NEMĚLO BY SE NIKDY STÁT!
		fprintf(stderr, "get_available_thread: Something is wrong, free thread index not found!\n");
		for (i = 0; i < num_threads; i++)
			fprintf(stderr, "get_available_thread: thread[%d].state == %d\n", i, threads[i].state);
		return -1;
		// exit(EXIT_FAILURE);
	}

	return i;
}

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
	int w = threads[thread_idx].w;

	work[w].thread_index = thread_idx;		// nastav id

	// synchronizace počátku výpočtu, již lze čekat na dokončení
	// thread_running

	// zahájení výpočtu
	if (verbose > 1)
		printf("    Vlákno %2d pro výpočet %2d zahájeno.\n", thread_idx, w+1);
	work[threads[thread_idx].w].state = WORK_STATE_BUSY;
	if (verbose < 1)	// pokud nevypisujeme výsledky, zobrazíme stavy
		print_states();
	do_work(threads[thread_idx].w);
	work[threads[thread_idx].w].state = WORK_STATE_DONE;
	if (verbose < 1)	// pokud nevypisujeme výsledky, zobrazíme stavy
		print_states();
	if (verbose > 1)
		printf("    Vlákno %2d pro výpočet %2d dokončeno.\n", thread_idx, w+1);

	return NULL;
}

void *results_thread(void *arg)
{
	int w;
	int rc;

	time_threads_sum = 0;
	for (w = 0; w < num_work; w++) {
		// čekání na začátek práce vlákna pro výpočet w
		// thread_running

		// uvolníme prostředky pro tento semafor
		// thread_running

		// vlákno pro výpočet w již běží, čekej na ukončení výpočtu
		rc = pthread_join(threads[work[w].thread_index].id, NULL);
		if (rc)
			fprintf(stderr, "Joining thread %d id 0x%lX failed: %s\n",
				w, threads[work[w].thread_index].id, strerror(rc));

		// uvolnění vlákna pro další výpočet
		threads[work[w].thread_index].state = THREAD_AVAILABLE;

		// signalizujeme volné vlákno
		// threads_available

		// tisk výsledků
		print_results(w);
		time_threads_sum += work[w].result;
	}

	return NULL;
}

int init_work()
{
	int w;

	// alokace pole pro výsledky výpočtů, automaticky nulováno
	work = calloc(num_work, sizeof(struct work_t));
	if (work == NULL) {
		perror("work calloc");
		exit(EXIT_FAILURE);
	}

	// inicializace pole pro jednotlivé výpočty
	for (w = 0; w < num_work; w++) {
		// inicializace semaforu pro synchronizaci
		// thread_running

		// inicializace stavu
		work[w].state = WORK_STATE_IDLE;
	}

	// inicializace semaforu volných vláken
	// threads_available

	// alokace a inicializace pole vláken na nuly (THREAD_AVAILABLE je 0)
	threads = calloc(num_threads, sizeof(struct threads_t));
	if (threads == NULL) {
		perror("threads calloc");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int release_resources()
{
	// semafory pro synchronizaci začátku jsou uvolněny ihned po dokončení výpočtu
	// uvolnění pole výpočtů
	free(work);

	// uvolnění pole vláken
	free(threads);

	// uvolnění semaforu evidujícího volná pracovní vlákna
	// threads_available

	return 0;
}

// vytváření vláken
void do_all()
{
	pthread_t	results_thread_id;
	int		w;
	int		rc;
	int		thread_index;

	// inicializace
	init_work();
	
	if (verbose > 1)
		printf("Spouští se vlákno pro sběr výsledků.\n");
		
	// vlákno na zpracování výsledků (jejich tisk)
	pthread_create(&results_thread_id, NULL, results_thread, NULL);

	// měříme čas
	time_start = get_time();

	if (verbose)
		printf("Výpočtů celkem:    %d\n"
		       "Max. počet vláken: %d\n", num_work, num_threads);
		
	// spouštění výpočtů
	for (w = 0; w < num_work; w++) {
		// alokuj vlákno pro výočet
		if ((thread_index = get_available_thread()) == -1) {
			exit(EXIT_FAILURE);
		}
		// nastav referenci na výsledky
		threads[thread_index].w = w;
		threads[thread_index].arg = thread_index;	// argument
		// vytvoření vlákna
		rc = pthread_create(&threads[thread_index].id, NULL,
			working_thread, &threads[thread_index].arg);
		if (rc) {
			fprintf(stderr, "\nCannot create thread: %s\n", strerror(rc));
			exit(EXIT_FAILURE);
		}
	}

	// čekáme na dokončení všech výpočtů
	rc = pthread_join(results_thread_id, NULL);

	time_total = get_time() - time_start;	// kolik času to trvalo celkem

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
