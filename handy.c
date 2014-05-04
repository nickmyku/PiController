/*
 * handy.c
 * Author: Anthony Cargile <acargile@embarkindustries.com>
 * App for manual control of 3D Printer interfaces (or Pi GPIO in general)
 * Copyright 2014 Embark Industries, all rights reserved.
 */
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <wiringPi.h>

#include "handy.h"
#include "slave/commands.h" /* for the pin definitions */

#define V_MAX (double)250.0 /* mm/min */
#define NSEC_DELAY 500000.0 /* Nanoseconds between signals */
#define pthread_yield_np() pthread_yield(NULL)

// globals:
double steps_per_mm = 1/.00635; // same for Y and X
pthread_t lim_thread;

int main(int argc, char **argv) {
	char c = ' ';
	int speed = 250;
	struct timespec ldelay;

	if(wiringPiSetup() == -1) {
		printf("Pin setup failed!\n(Do you even root, bro?)\n");
		exit(1);
	}

	// set up pins:
	pinMode(X_PIN, OUTPUT);
	pinMode(Y_PIN, OUTPUT);
	pinMode(Z_PIN, OUTPUT);
	pinMode(X_DIR_PIN, OUTPUT);
	pinMode(Y_DIR_PIN, OUTPUT);
	pinMode(Z_DIR_PIN, OUTPUT);
	pinMode(LASER_PIN, OUTPUT);
	pinMode(LIM_X, INPUT);
	pinMode(LIM_Y, INPUT);
	pinMode(LIM_Z, INPUT);
/*
	// set up watchdog thread (limit switch detection)
	int rc = pthread_create(&lim_thread, NULL, lim_watchdog, (void *)NULL);
	if(rc) {
		printf("Could not allocate a thread for the Y-axis.\nProposed resolution: swallow cyanide.");
		pthread_exit(NULL);
	}
*/
	initscr(); /* Start curses mode */
	noecho(); /* Suppress outputting of characters as pressed */
	while(c != 'q') {
		printw("Handy Control System\n\n"); /* Print Header */
		printw("Movement Speed: %d\n", speed);
		printw("Recieved key: %c\n\n", c);
		printw("Key command reference:\ns - set speed\nq - quit\nu - X motor UP\nj - X motor DOWN\ni - Y motor UP\nk - Y motor DOWN\nl - fire laser (pew pew!)");
		refresh(); /* Print it on to the real screen */
		c = getch(); /* Wait for user input */
		switch(c) {
			case 'q':
				/* Fall through to escape while() loop */
				break;
			case 's':
				clear();
				printw("Speed setter\n\nSet speed in mm/min here (between 0 and 500, please)\nDesired speed: ");
				char buf[10];
				char index = 0;
				speed = -1;
				char cc;
				echo();
				while(1) {
					cc = getch();
					if(cc == '\n' || cc == '\r') break;
					buf[index] = cc;
					index++;
				}
				buf[index] = '\0'; // terminate string
				speed = (int)strtol(buf, NULL, 0);
				clear();
				printw("New speed set to %d mm/min\nPress the any key to continue. . .", speed);
				getch(); // good old DOS humor
				noecho();
			case 'u':
				digitalWrite(X_DIR_PIN, 1);
				move(X_PIN, speed);
				break;
			case 'j':
				digitalWrite(X_DIR_PIN, 0);
				move(X_PIN, speed);
				break;
			case 'i':
				digitalWrite(Y_DIR_PIN, 1);
				move(Y_PIN, speed);
				break;
			case 'k':
				digitalWrite(Y_DIR_PIN, 0);
				move(Y_PIN, speed);
				break;
			case 'l':
    				ldelay.tv_nsec = 500; // tweak as needed
				digitalWrite(LASER_PIN, 1);
				nanosleep(&ldelay,NULL);
				digitalWrite(LASER_PIN, 0);
				break;
			default:
				printw("Caught: %c\n", c);
				break;
		}
		clear();
	}
	endwin(); /* End curses mode */
	return EXIT_SUCCESS;
}

void move_pin(int pin, int speed) {
	struct timespec constdelay;
	struct timespec delay;
	double temp_time;
	double nano_time;
	double steps;
	int i;

	constdelay.tv_nsec = (long)NSEC_DELAY; // .5s, to stagger state changes
	temp_time = 1/((speed*steps_per_mm)/60000);	//calculate ms delay
	nano_time = ((temp_time*1000000)/2); //convert to nanoseconds then divide by 2 (50% duty)
	delay.tv_nsec = (long)nano_time;
	for(i = 0; i <= 10; i++) {
		digitalWrite(pin, 1);
            	nanosleep(&constdelay,NULL);
            	digitalWrite(pin, 0);
            	nanosleep(&delay,NULL);
	}
}

/*
 * Watch over limit switches and yank the plug if one trips
 */
void *lim_watchdog(void* ignored) {
    struct timespec watchdelay;
    watchdelay.tv_nsec = 50000; // tweak as needed
    
    while(1) {
        nanosleep(&watchdelay,NULL);
        if(digitalRead(LIM_X) != 1 || digitalRead(LIM_Y) != 1) {
	    endwin(); // remove this if you're brave and/or stupid
            printf("LIMIT SWITCH DEPRESSED; SYSTEM SHUTTING DOWN COMMIT SEPUKKU NOW!!!");
            exit(1);
            pthread_exit(NULL);
        }
        pthread_yield_np();
    }
}
