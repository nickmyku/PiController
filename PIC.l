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
#include "y.PIC.h"
#include "slave/commands.h"

#define DEBUG_FLAG 1
#define COMPILED_FILE "/tmp/PIC.o"

#ifndef TEST_BUILD
// building on non-Pi, include dummy GPIO interface:
#include "gpio_dummy.h"
#else
// real deal, include real GPIO library files:
#include <wiringPi.h>
#include <wiringPiI2C.h>
#endif

// use this to provide dev-only feedback. See printDebug() too. 
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)

// globals (don't hate):
uint32_t daughter = 0x0; // daughter board shift register
FILE *fcomp; // compiled output (/tmp file)
int i2cfile;
double pos_x = -1;
double pos_y = -1;
double pos_z = -1;
double X_limit = -1; // X-axis time (in ms) from 0 to its limit at full power
double Y_limit = -1; // " " same for Y-axis
int dline = 0;

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
<MOVEMENT>{newline} {dline++;axis_updated();BEGIN INITIAL;}

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
	calibrate();
	yylex();
	printOut();
	return EXIT_SUCCESS;
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
int i2cinit() {
    int err = 0;
    
    i2cfile = wiringPiI2CSetup(SLAVE_BUS_ID);
    if(i2cfile == -1) err++;
    DEBUG("Master bus initialized, or should be (%d)", err);
    
    // initialize the bus with the "hello" handshake:
    int op = OP_HELLO;
    if(write(i2cfile, &op, sizeof(int)) < 0) err++;
    sleep(1);
    if(read(i2cfile, &op, sizeof(int))) err++;
    if(op != OP_HELLO) {
        err++;
        DEBUG("Slave said: %d", op);
    }
    DEBUG("Handshake results: %d, circle-jerk status", err);
    
    if(err == 0) return 0;
    // else
    DEBUG("%d errors occurred while initializing the i2c bus.\nHave you considered stripping as a profession instead?", err);
    return -1;
}

/*
 * Prints the actual deliverable, using compiled Gcode generated via Lex
 */
int printOut() {
    if(i2cinit() == -1) {
        eprintf("i2c failed to initialize.\nTake a shot every time you see this.");
        return -1;
    }
    
	return 0;
}

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
    fputc('\n', stderr);
    
    va_end(list);
 }
 
/*
 * Called whenever the armature is to be moved (pos_x-z updated)
 */
void axis_updated() {
	
	pos_x = pos_y = pos_z = -1;
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
