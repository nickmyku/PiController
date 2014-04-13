%{
/*
 ============================================================================
 Name        : PIC.c
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
#include "y.PIC.h"
#include "slave/commands.h"

#define DEBUG_FLAG 1
#define COMPILED_FILE "/tmp/PIC.o"
#define V_MAX 100 /* mm/min */
#define NSEC_DELAY 500000 /* Nanoseconds between signals */

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

// globals (don't hate):
FILE *fcomp; // compiled output (/tmp file)
int i2cfile;
double pos_x = 0;
double pos_y = 0;
double pos_z = 0;
double last_pos_x = 0;
double last_pos_y = 0;
double last_pos_z = 0;
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

<INITIAL>{general}"0"?"1" {BEGIN MOVEMENT;}
<INITIAL>{misc}"0"?"3"{whitespace}*{newline}+{misc}"0"?"5" ;
<INITIAL>{misc}"0"?"3" {laseroff();}
<INITIAL>{misc}"0"?"5" {laseron();}
<INITIAL>{misc}"0"?"2" {yyterminate();}

<MOVEMENT>{xaxis}{digit}+{point}{digit}+ {yyless(1);BEGIN X;}
<MOVEMENT>{yaxis}{digit}+{point}{digit}+ {yyless(1);BEGIN Y;}
<MOVEMENT>{zaxis}{digit}+{point}{digit}+ {yyless(1);BEGIN Z;}
<MOVEMENT>{faxis}{digit}+{point}{digit}+ ;
<MOVEMENT>{newline} {dline++;DEBUG("Axis Updated (%d)", dline);axis_updated();BEGIN INITIAL;}

<X>{digit}+{point}{digit}+ {pos_x = strtod(yytext, NULL);DEBUG("X set to %f",pos_x);BEGIN MOVEMENT;}
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
	if(wiringPiSetup() == -1) {
		eprintf("Pin setup failed!\n(Do you even root, bro?)\n");
		exit(1);
	}

	if(argc <= 1) {
		eprintf("Your argument is invalid.\nAnd you're a towel.\n");
		exit(1);
	}
	
	FILE *fp = NULL;
	if((fp = fopen(argv[1], "r")) == NULL) {
		eprintf("That file doesn't even exist.\nNice try, asshole.\n");
		exit(1);
	}
	yyin = fp;
	
	if((fcomp = fopen(COMPILED_FILE, "wb")) == NULL) {
		eprintf("Even Helen Keller knows you should be able to write to /tmp, and she's fucking dead.\n");
		exit(1);
	}
    
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
 * Initialize the i2c bus the a master node
 */
//int i2cinit() {
//    int err = 0;
//    
//    i2cfile = wiringPiI2CSetup(SLAVE_BUS_ID);
//    if(i2cfile == -1) err++;
//    DEBUG("Master bus initialized, or should be (%d)", err);
//    
//    // initialize the bus with the "hello" handshake:
//    int op = OP_HELLO;
//    if(write(i2cfile, &op, sizeof(int)) < 0) err++;
//    sleep(1);
//    if(read(i2cfile, &op, sizeof(int))) err++;
//    if(op != OP_HELLO) {
//        err++;
//        DEBUG("Slave said: %d", op);
//    }
//    DEBUG("Handshake results: %d, circle-jerk status", err);
//    
//    if(err == 0) return 0;
//    // else
//    DEBUG("%d errors occurred while initializing the i2c bus.\nHave you considered stripping as a profession instead?", err);
//    return -1;
//}

/*
 * Prints the actual deliverable, using compiled Gcode generated via Lex
 */
//int printOut() {
////    if(i2cinit() == -1) {
////        eprintf("i2c failed to initialize.\nTake a shot every time you see this.");
////        return -1;
////    }
//    
//	return 0;
//}

/*
 * Prettyful debug function 
 * (DO NOT CALL THIS DIRECTLY - use DEBUG() instead, or else tiny elves will feast on your toes)
 */
void printDebug(const char *fmt, const int line, const char *file, ...) {
    va_list list;
    va_start(list, file);

    if(DEBUG_FLAG) {
      fprintf(stderr, "Debug: on line %d of file %s: ", line, file);
      vfprintf(stderr, fmt, list);
      putc('\n',stderr);
    }
    va_end(list);
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
//double coef_x; // how many more X than Y
//double coef_y; // Y than X
//double coef_threshold; // write when reached
void *simul_y(void *targv) {
    int i;
    int *argv = (int *)targv;
    DEBUG("Y thread initiated");
    for(i = 0; i <= argv[0]; i++) {
        DEBUG("Writing Y signals...");
        digitalWrite(Y_PIN, 1);
        nanosleep(&constdelay,NULL);
        digitalWrite(Y_PIN, 0);
        nanosleep(&ydelay,NULL);
        pthread_yield_np(); 
    }
    
    pthread_exit(NULL);
}

/*
 * Called whenever the armature is to be moved (pos_x-z updated)
 */
void axis_updated() {
    double dx;
    double dy;
    double dz;
    double steps_x;
    double steps_y;
    double steps_z;
    int i;
    
    DEBUG("From: <%f,%f,%f> to <%f,%f,%f>",last_pos_x, last_pos_y, last_pos_z, pos_x, pos_y, pos_z);
    
    dx = pos_x - last_pos_x;
    dy = pos_y - last_pos_y;
    dz = pos_z - last_pos_z;
    last_pos_x = pos_x;
    last_pos_y = pos_y;
    last_pos_z = pos_z;
    pos_x = pos_y = pos_z = -1;
    
    steps_x = sqrt((double)((V_MAX * V_MAX)*60) / (1 + ((dy*dy)/(dx*dx)))); // mm/sec
    steps_y = sqrt((double)((V_MAX * V_MAX)*60) / (1 + ((dx*dx)/(dy*dy)))); // mm/sec
    constdelay.tv_nsec = NSEC_DELAY; // .5s, to stagger state changes
    xdelay.tv_nsec = ((steps_x > steps_y)?(abs(abs(steps_y)/abs(steps_x))):1)*NSEC_DELAY;
    ydelay.tv_nsec = ((steps_x < steps_y)?(abs(abs(steps_x)/abs(steps_y))):1)*NSEC_DELAY;
    
    DEBUG("Moving X %f steps at %f mm/s (d. %f) and Y %f steps at %f mm/s (d. %f)",dx,steps_x,xdelay.tv_nsec,dy,steps_y,ydelay.tv_nsec);
    
    // directional pins
    if(dx > 0) {
        digitalWrite(X_DIR_PIN, 1);
    }
    else {
        digitalWrite(X_DIR_PIN, 0);
    }
    if(dy > 0) {
        digitalWrite(Y_DIR_PIN, 1);
    }
    else {
        digitalWrite(Y_DIR_PIN, 0);
    }
    
    if(steps_y) {
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
    if(steps_x != 0) {
        for(i = 0; i <= steps_x; i++) {
            DEBUG("Writing X signals...");
            digitalWrite(X_PIN, 1);
            nanosleep(&constdelay,NULL);
            digitalWrite(X_PIN, 0);
            nanosleep(&xdelay,NULL);
            pthread_yield_np(); 
        }
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
