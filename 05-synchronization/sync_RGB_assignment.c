// Operating Systems: sample code  (c) Tomáš Hudec
// Synchronization

// Modified: 2015-12-17

// Assignment:
//
// The program must print correctly initialized RGB value.
// Synchronize using POSIX semaphore.
// Do not modify the thread_set() function.

// Zadání:
//
// Program musí vypsat správně inicializovanou hodnotu RGB.
// Synchronizujte použitím posixového semaforu.
// Funkci thread_set() nemodifikujte.

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

// declaration of synchronization resource


// color type
typedef struct RGB {
	int	r, g, b;		// color
} RGB_t;

// shared variable
volatile RGB_t rgb = { .r = -1, .g = -1, .b = -1 };	// default value is undefined color

int sleep_time = 0;			// sleeping time (simulation of work)

// all color components must be between 0 and 255 inclusive
bool isColorValid(int r, int g, int b)
{
	return r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255;
}

// print by the rgb components
void printRGB(void)
{
	// print must be executed AFTER assigning the component values
	if (isColorValid(rgb.r, rgb.g, rgb.b))
		printf("The color = rgb(%d, %d, %d) = #%02X%02X%02X\n",
			rgb.r, rgb.g, rgb.b, rgb.r, rgb.g, rgb.b);
	else
		fprintf(stderr, "The color has one or more invalid compnent values: %d, %d, %d\n",
			rgb.r, rgb.g, rgb.b);
	return;
}

// set the rgb components
bool setRGB(int r, int g, int b)
{
	bool ret = false;

	// check the validity
	if (isColorValid(r, g, b)) {
		// assigning the values must be done BEFORE printing
		rgb.r = r;
		rgb.g = g;
		rgb.b = b;
		ret = true;
	}

	return ret;
}

// 2nd thread function (the 1st one beeing main())
void *thread_set(void *arg)
{
	sleep(sleep_time);		// simulate work
	setRGB(0xFF, 0x55, 0x00);	// set color components
	return NULL;
}

// 1st thread function
int main(int argc, char *argv[])
{
	pthread_t thread_id;
	int rc;

	// id

	// initialize synchronization resource


	if (argc > 1)			// if the argument is given, use it as sleep time value
		sleep_time = atoi(argv[1]);

	// create a thread that initializes the color
	rc = pthread_create(&thread_id, NULL, thread_set, NULL);
	if (rc) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}

	printRGB();			// print the color value

	(void) pthread_join(thread_id, NULL);

	// release resources


	return EXIT_SUCCESS;
}
