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
#include "slave/commands.h" 		/* for the pin definitions  */
#include "drivers/axis_config.h"	/* defines motor parameters */
#include "drivers/sx1509.h"		/* driver for W axis and LEDs*/

#define V_MAX (double)250.0 /* mm/min */
#define NSEC_DELAY 500000.0 /* Nanoseconds between signals */
#define pthread_yield_np() pthread_yield(NULL)

// globals:
double steps_per_mm_x;
double steps_per_mm_y;
double steps_per_mm_z;
double steps_per_mm_v;
pthread_t lim_thread;

// SX1509 Driver:
int sx = -1;

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
	pinMode(V_PIN, OUTPUT);
	pinMode(X_DIR_PIN, OUTPUT);
	pinMode(Y_DIR_PIN, OUTPUT);
	pinMode(Z_DIR_PIN, OUTPUT);
	pinMode(V_DIR_PIN, OUTPUT);
	pinMode(LASER_PIN, OUTPUT);
	pinMode(LIM_X, INPUT);
	pinMode(LIM_Y, INPUT);
	pinMode(LIM_Z, INPUT);
	pinMode(LIM_V, INPUT);
	
	//for good measure, turn off laser
	digitalWrite(LASER_PIN, 0);
	
	// set up driver(s):
    	if((sx = initializeDevice()) == -1) {
        	printf("\"Fatal issue involving the driver\"\n-Princess Diana");
        	pthread_exit(NULL);
    	}
	
	//set up motor paramters
	steps_per_mm_x = stepsPerMM('x');
	steps_per_mm_y = stepsPerMM('y');
	steps_per_mm_z = stepsPerMM('z');
	steps_per_mm_v = stepsPerMM('v');
	
	// set up watchdog thread (limit switch detection)
	int rc = pthread_create(&lim_thread, NULL, lim_watchdog, (void *)NULL);
	if(rc) {
		printf("Could not allocate a thread for the Y-axis.\nProposed resolution: swallow cyanide.");
		pthread_exit(NULL);
	}
	
	LEDColor(sx, 0, 255, 0, 0);	//turn LEDs red
	LEDColor(sx, 1, 255, 0, 0);

	initscr(); /* Start curses mode */
	noecho(); /* Suppress outputting of characters as pressed */
	while(c != 'q') {
		printw("Handy Control System\n\n"); /* Print Header */
		printw("Movement Speed: %d mm/min\n", speed);
		printw("Recieved key: %c\n\n", c);
		printw("Key command reference:\ne - set speed\nq - quit\n\nw - X motor UP\ns - X motor DOWN\nd - Y motor UP\na - Y motor DOWN\n\nr - Z motor UP\nf - Z motor DOWN\nt - V motor UP\ng - V motor DOWN\n\nZ - W axis minimum limit\nX - W axis maximum limit\n\nl - fire laser (pew pew!)");
		refresh(); /* Print it on to the real screen */
		c = getch(); /* Wait for user input */
		switch(c) {
			case 'q':
				/* Fall through to escape while() loop */
				break;
			case 'e':
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
			case 'w':						//had to change to WASD keys, played too many pc games as a kid
				digitalWrite(X_DIR_PIN, 1);
				move_pin(X_PIN, speed, steps_per_mm_x);
				break;
			case 's':
				digitalWrite(X_DIR_PIN, 0);
				move_pin(X_PIN, speed, steps_per_mm_x);
				break;
			case 'd':
				digitalWrite(Y_DIR_PIN, 1);
				move_pin(Y_PIN, speed, steps_per_mm_y);
				break;
			case 'a':
				digitalWrite(Y_DIR_PIN, 0);
				move_pin(Y_PIN, speed, steps_per_mm_y);
				break;
			case 'r':
				digitalWrite(Z_DIR_PIN, 1);
				move_pin(Z_PIN, speed, steps_per_mm_z);
				break;
			case 'f':
				digitalWrite(Z_DIR_PIN, 0);
				move_pin(Z_PIN, speed, steps_per_mm_z);
				break;
			case 't':
				digitalWrite(V_DIR_PIN, 1);
				move_pin(V_PIN, speed, steps_per_mm_v);
				break;
			case 'g':
				digitalWrite(V_DIR_PIN, 0);
				move_pin(V_PIN, speed, steps_per_mm_v);
				break;
			case 'z':
				LEDColor(sx, 0, 255, 255, 0);	//turn LEDs yellow
				LEDColor(sx, 1, 255, 255, 0);
				moveToLimit(sx, FALSE, 255, TRUE);
				LEDColor(sx, 0, 255, 0, 0);	//turn LEDs back to red
				LEDColor(sx, 1, 255, 0, 0);
				break;
			case 'x':
				LEDColor(sx, 0, 255, 255, 0);	//turn LEDs yellow
				LEDColor(sx, 1, 255, 255, 0);
				moveToLimit(sx, TRUE, 255, TRUE);
				LEDColor(sx, 0, 255, 0, 0);	//turn LEDs back to red
				LEDColor(sx, 1, 255, 0, 0);
				break;
			case 'l':
				LEDColor(sx, 0, 0, 0, 255);	//turn LEDs blue
				LEDColor(sx, 1, 0, 0, 255);
    				ldelay.tv_nsec = 500; // tweak as needed
				digitalWrite(LASER_PIN, 1);
				nanosleep(&ldelay,NULL);
				digitalWrite(LASER_PIN, 0);
				LEDColor(sx, 0, 255, 0, 0);	//turn LEDs back to red
				LEDColor(sx, 1, 255, 0, 0);
				break;
			default:
				printw("Caught: %c\n", c);
				break;
		}
		clear();
	}
	LEDColor(sx, 0, 0, 0, 0);	//turn off LEDS
	LEDColor(sx, 1, 0, 0, 0);
	endwin(); /* End curses mode */
	return EXIT_SUCCESS;
}

void move_pin(int pin, int speed, double steps_per_mm) {
	struct timespec constdelay;
	struct timespec delay;
	double temp_time;
	double nano_time;
	double steps;
	int i;
	int num_steps = 1;
	
	//dynamically caluclate the number of loop iterations depending on the desired move speed
	//this should fix the lag time expericed between key lift and motor stop at low speeds
	if(speed > 16)
	{
		num_steps = (int)(speed/15);
	}

	constdelay.tv_nsec = (long)NSEC_DELAY; // .5s, to stagger state changes
	temp_time = 1/((speed*steps_per_mm)/60000);	//calculate ms delay
	nano_time = ((temp_time*1000000)/2); //convert to nanoseconds then divide by 2 (50% duty)
	delay.tv_nsec = (long)nano_time;
	for(i = 0; i <= num_steps; i++) {
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

/*
 * Prettyful debug function 
 * (DO NOT CALL THIS DIRECTLY - use DEBUG() instead, or else tiny elves will feast on your toes)
 */
void printDebug(const char *fmt, const int line, const char *file, ...) {
#ifdef DEBUG_FLAG
    va_list list;
    va_start(list, file);

    fprintf(stderr, "Debug: on line %d of file %s: ", line, file);
    vfprintf(stderr, fmt, list);
    putc('\n',stderr);
    
    va_end(list);
#endif /* DEBUG_FLAG */
}
