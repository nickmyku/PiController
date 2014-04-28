%{
/*
 ============================================================================
 Name        : PIC.l
 Author      : Anthony Cargile
 Version     :
 Copyright   : Copyright 2014 Embark Industries
 Description : Entry point for PIC manufacturing utility.
 ============================================================================
 */

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
    
#include "y.PIC.h"
#include "slave/commands.h"
#include "drivers/sx1509.h"
    
#ifndef TEST_BUILD
// building on non-Pi, include dummy GPIO interface:
#include "gpio_dummy.h"
#else
// real deal, include real GPIO library files:
#include <wiringPi.h>
#include <wiringPiI2C.h>
#define pthread_yield_np() pthread_yield(NULL)
#endif

// use this to provide dev-only feedback. See printDebug() too. 
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)
    
#define COMPILED_FILE "/tmp/PIC.o"
#define V_MAX (double)250.0 /* mm/min */
#define NSEC_DELAY 500000.0 /* Nanoseconds between signals */

// globals (don't hate):
FILE *fcomp; // compiled output (/tmp file)
int i2cfile;
    
// SX1509 Driver:
int sx = -1;
    
// position storage (shared pthread memory)
double pos_x = 0;
double pos_y = 0;
double pos_z = 0;
double last_pos_x = 0;
double last_pos_y = 0;
double last_pos_z = 0;
double x_steps_per_mm = 1/.00635; 	//math to get this number is defined elswhere
double y_steps_per_mm = 1/.00635;
int dline = 0;
char x_ready = 0;
char y_ready = 0;
pthread_t lim_thread;

%}

%s MOVEMENT
%s X
%s Y
%s Z

letter [a-zA-Z]
digit [0-9]
newline \n
whitespace [ \t]+

misc "M"
general "G"
xaxis "X"
yaxis "Y"
zaxis "Z"
faxis "F"
point "."

%%

<INITIAL>{general}"0"?"1" {/* M[0]1 */BEGIN MOVEMENT;}
<INITIAL>{misc}"0"?"3"{whitespace}*{newline}+{misc}"0"?"5" ;
<INITIAL>{misc}"0"?"3" {/* M[0]3 */laseroff();}
<INITIAL>{misc}"0"?"5" {/* M[0]5 */laseron();}
<INITIAL>{misc}"1""0" {/* M10 */moveToLimit(sx, false, 200, true);}
<INITIAL>{misc}"1""1" {/* M11 */moveToLimit(sx, true, 200, true);}
<INITIAL>{misc}"0"?"2" {/* M[0]2 */yyterminate();}

<MOVEMENT>{xaxis}{digit}+{point}{digit}+ {/* M[0]1 X#.# */yyless(1);BEGIN X;}
<MOVEMENT>{yaxis}{digit}+{point}{digit}+ {/* M[0]1 Y#.# */yyless(1);BEGIN Y;}
<MOVEMENT>{zaxis}{digit}+{point}{digit}+ {/* M[0]1 Z#.# */yyless(1);BEGIN Z;}
<MOVEMENT>{faxis}{digit}+{point}{digit}+ ;
<MOVEMENT>{newline} {/* EOL of M[0]1 */dline++;sleep(3);DEBUG("Axis Updated (%d)", dline);axis_updated();BEGIN INITIAL;}

<X>{digit}+{point}{digit}+ {pos_x = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Y>{digit}+{point}{digit}+ {pos_y = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Z>{digit}+{point}{digit}+ {pos_z = strtod(yytext, NULL);BEGIN MOVEMENT;}
<X,Y,Z>{whitespace}+ {DEBUG("What, did Charlie Sheen write this shit? (line %d)", dline);BEGIN MOVEMENT;}

<INITIAL,X,Y,Z>{newline} {dline++;}
<MOVEMENT,INITIAL>{whitespace}+ ;
<INITIAL,MOVEMENT,X,Y,Z>. {DEBUG("I didn't sign up for this: '%s' (line %d)", yytext, dline);}

%%

/*
 * THE entry point - keep it pure, kids. Just like your heroin.
 */
int main(int argc, char **argv) {
	sanity(argc, argv);
	yylex();
    cleanup();
    pthread_exit(NULL);
	exit(EXIT_SUCCESS);
}

/*
 * Rudimentary pre-execution...stuff.
 */
int sanity(int argc, char **argv) {
    // setup core components:
	if(wiringPiSetup() == -1) {
		eprintf("Pin setup failed!\n(Do you even root, bro?)\n");
		pthread_exit(NULL);
	}

	if(argc <= 1) {
		eprintf("Your argument is invalid.\nAnd you're a towel.\n");
		pthread_exit(NULL);
	}
	
	FILE *fp = NULL;
	if((fp = fopen(argv[1], "r")) == NULL) {
		eprintf("That file doesn't even exist.\nNice try, asshole.\n");
		pthread_exit(NULL);
	}
	yyin = fp;
	
	if((fcomp = fopen(COMPILED_FILE, "wb")) == NULL) {
		eprintf("Even Helen Keller knows you should be able to write to /tmp, and she's fucking dead.\n");
		pthread_exit(NULL);
	}
    
    // set up pins:
    pinMode(X_PIN, OUTPUT);
    pinMode(Y_PIN, OUTPUT);
    pinMode(Z_PIN, OUTPUT);
    pinMode(X_DIR_PIN, OUTPUT);
    pinMode(Y_DIR_PIN, OUTPUT);
    pinMode(Z_DIR_PIN, OUTPUT);
    pinMode(LASER_PIN, PWM_OUTPUT);
    pinMode(LIM_X, INPUT);
    pinMode(LIM_Y, INPUT);
    pinMode(LIM_Z, INPUT);
    
    // set up driver(s):
    if((sx = initializeDevice()) == -1) {
        eprintf("\"Fatal issue involving the driver\"\n-Princess Diana");
        pthread_exit(NULL);
    }
    
    // set up watchdog thread (limit switch detection)
    int rc = pthread_create(&lim_thread, NULL, lim_watchdog, (void *)NULL);
    if (rc){
        eprintf("Could not allocate a thread for the Y-axis.\nProposed resolution: swallow cyanide.");
        pthread_exit(NULL);
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
            eprintf("LIMIT SWITCH DEPRESSED; SYSTEM SHUTTING DOWN COMMIT SEPUKKU NOW!!!");
            exit(1);
            pthread_exit(NULL);
        }
        pthread_yield_np();
    }
}

/*
 * Calibrate the machine and sense timing values, for later
 */
void calibrate() {
	return;
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

/*
 * Error function (called regardless of DEBUG flag to let user know they fucked up)
 */
void eprintf(const char *fmt, ...) {
 	va_list list;
    va_start(list, fmt);
    char *prefix = "ERROR: ";
    
    fprintf(stderr, prefix);
    vfprintf(stderr, fmt, list);
#warning ignore the "format not a string literal" bullshit
    fputc('\n', stderr);
    
    va_end(list);
 }

/*
 * Thread entry point for moving the Y axis simultaneously with the X axis
 */
struct timespec constdelay;
struct timespec xdelay;
struct timespec ydelay;
double y_writes;
double x_writes;
double steps_x;
double steps_y;
void *simul_y(void *targv) {
    double i;
    int *argv = (int *)targv;
    DEBUG("Y thread initiated");
    for(i = 0.0; i <= fabs(steps_y); i++) {
        digitalWrite(Y_PIN, 1);
        nanosleep(&ydelay,NULL);
        digitalWrite(Y_PIN, 0);
        nanosleep(&ydelay,NULL);
        y_writes++;
        pthread_yield_np(); 
    }
    steps_y = 0.0; // allows X/main thread to proceed
    DEBUG("Wrote %f Y signals",y_writes);
    
    pthread_exit(NULL);
}

/*
 * Called whenever the armature is to be moved (pos_x-z updated)
 */
void axis_updated() {
    double dx;
    double dy;
    double dz;
    double steps_z;
    double x_vel;
    double y_vel;
    double temp_x_time;
    double temp_y_time;
    double nano_x_time;
    double nano_y_time;
    double i;
    
    DEBUG("\nFrom: <%f,%f,%f> to <%f,%f,%f>",last_pos_x, last_pos_y, last_pos_z, pos_x, pos_y, pos_z);
    
    dx = pos_x - last_pos_x;
    dy = pos_y - last_pos_y;
    dz = pos_z - last_pos_z;
    last_pos_x = pos_x;
    last_pos_y = pos_y;
    last_pos_z = pos_z;
    pos_x = pos_y = pos_z = -1.0;
    x_writes = y_writes = 0.0;
    
    steps_x = dx*x_steps_per_mm;
    steps_y = dy*y_steps_per_mm;
    
    DEBUG("dX: %f, dY: %f",dx,dy);
        
    if((dx != 0) && (dy != 0))
    {
    	x_vel = sqrt((V_MAX*V_MAX)/(1+((dy*dy)/(dx*dx))));
    	y_vel = sqrt((V_MAX*V_MAX)/(1+((dx*dx)/(dy*dy))));
        
    }
    else
    {
    	x_vel = V_MAX;
    	y_vel = V_MAX;
    }
    
    temp_x_time = 1/((x_vel*x_steps_per_mm)/60000);	//calculate ms delay for x step
    temp_y_time = 1/((y_vel*y_steps_per_mm)/60000);	//calculate ms delay for y step
    
    nano_x_time = ((temp_x_time*1000000)/2); //convert to nanoseconds then divide by 2 (50% duty)
    nano_y_time = ((temp_y_time*1000000)/2); //convert to nanoseconds then divide by 2 (50% duty)
    
    xdelay.tv_nsec = (long)nano_x_time;
    ydelay.tv_nsec = (long)nano_y_time;
    constdelay.tv_nsec = NSEC_DELAY; // .5s, to stagger state changes
        
    DEBUG("\nX %f steps at %f mm/s (d. %ld)\nY %f steps at %f mm/s (d. %ld)", steps_x, x_vel,xdelay.tv_nsec,steps_y,y_vel,ydelay.tv_nsec);
    
    // directional pins
    if(dx >= 0.0) {
        digitalWrite(X_DIR_PIN, 1);
    }
    else {
        digitalWrite(X_DIR_PIN, 0);
    }
    if(dy >= 0.0) {
        digitalWrite(Y_DIR_PIN, 1);
    }
    else {
        digitalWrite(Y_DIR_PIN, 0);
    }
    
    if(steps_y != 0.0) {
        pthread_t y_thread;
        int rc;
        int argv[2];
        
        argv[0] = steps_y;
        argv[1] = 1; // time delay between signals, to change speed...
        rc = pthread_create(&y_thread, NULL, simul_y, (void *)argv);
        if (rc){
            eprintf("Could not allocate a thread for the Y-axis.\nProposed resolution: swallow cyanide.");
            pthread_exit(NULL);
        }
    }
    
    if(steps_x != 0.0) {
        for(i = 0.0; i <= fabs(steps_x); i++) {
            digitalWrite(X_PIN, 1);
            nanosleep(&xdelay,NULL);
            digitalWrite(X_PIN, 0);
            nanosleep(&xdelay,NULL);
            x_writes++;
            pthread_yield_np(); 
        }
        DEBUG("Wrote %f X signals",x_writes);
    }
    
    if(steps_y != 0.0) {
        // wait for Y movement to finish
        do {
            pthread_yield_np();
        }
        while(steps_y != 0.0);
    }
}

/*
 * Sets the laser to the PWM-induced power level. Pew pew
 */
int laser(double power) {
	// leave this in, it has a legitimate purpose I promise
	if(ON_A_SHARK_HEAD) {
		eprintf("Kiss my black ass, Dr. Evil. Mr. Bigglesworth too!");
		return -1;
	}
	
	return 0;
}

/*
 * Last thing we do - close files, sync FS, turn off pins etc.
 */
void cleanup() {
    pthread_cancel(lim_thread);
    digitalWrite(X_PIN,0);
    digitalWrite(Y_PIN,0);
    digitalWrite(Z_PIN,0);
    digitalWrite(X_DIR_PIN,0);
    digitalWrite(Y_DIR_PIN,0);
    digitalWrite(Z_DIR_PIN,0);
    digitalWrite(LASER_PIN,0);
    fclose(fcomp);
    system("sync");
}
