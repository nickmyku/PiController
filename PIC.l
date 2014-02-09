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
#include <inttypes.h>
#include <sys/stat.h>
#include "y.PIC.h"

#define DEBUG_FLAG 1

#ifndef TEST_BUILD
// building on non-Pi, include dummy GPIO interface:
#include "gpio_dummy.h"
#else
// real deal, include real GPIO library files:
#include <wiringPi.h>
#endif

// use this to provide dev-only feedback. See printDebug() too. 
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)

// globals (don't hate):
double pos_x = -1; // these are all global state variables for positions
double pos_y = -1;
double pos_z = -1;
double last_pos_x = 0;
double last_pos_y = 0;
double last_pos_z = 0;

FILE *fcomp; // compiled output (/tmp file)
double X_limit = -1; // X-axis time (in ms) from 0 to its limit at full power
double Y_limit = -1; // " " same for Y-axis

/*
 * This is the internal state variable for the daughterboard's shift register.
 * It is a 32-bit unsigned binary number with bits as follows:
 * 
 * Bit : Description
 * 0   : X step pin
 * 1   : X direction pin
 * 2   : Y step pin
 * 3   : Y dir pin
 * 4-7 : (reserved)
 * 8   : Laser pin (PWM for power control) 
 * 9-31: (reserved)
 *
 * The limit switches will be read separately, likely OR'd together to a 
 * single dedicated input pin until a serial-bus solution is designed later.
 */
uint32_t daughter = 0x0;

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
<MOVEMENT>{newline} {axis_updated();BEGIN INITIAL;}

<X>{digit}+{point}{digit}+ {pos_x = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Y>{digit}+{point}{digit}+ {pos_y = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Z>{digit}+{point}{digit}+ {pos_z = strtod(yytext, NULL);BEGIN MOVEMENT;}
<X,Y,Z>{whitespace}+ {DEBUG("What, did Charlie Sheen write this shit?");BEGIN MOVEMENT;}

<MOVEMENT,INITIAL>{whitespace}+ ;
<INITIAL>{general}{digit}+ {DEBUG("This unknown general is a major pain: %s", yytext);}
<INITIAL>{misc}{digit}+ {DEBUG("I NEED AN ADULT: %s", yytext);}
<INITIAL,MOVEMENT,X,Y,Z>. {DEBUG("I didn't sign up for this: '%s'", yytext);}

%%

/*
 * THE entry point - keep it pure, kids. Just like your heroin.
 */
int main(int argc, char **argv) {
	sanity(argc, argv);
	calibrate();
	yylex();
	printOut();
	cleanup();
	return EXIT_SUCCESS;
}

/*
 * Rudimentary pre-execution...stuff.
 */
int sanity(int argc, char **argv) {
	if(wiringPiSetup() == -1) {
		eprintf("Pin setup failed!\n(Do you even root, bro?)");
		exit(1);
	}

	if(argc <= 1) {
		eprintf("Your argument is invalid.\nAnd you're a towel.");
		exit(1);
	}
	
	FILE *fp = NULL;
	if((fp = fopen(argv[1], "r")) == NULL) {
		eprintf("That file doesn't even exist.\nNice try, asshole.");
		exit(1);
	}
	yyin = fp;
	
	if((fcomp = fopen("/tmp/PIC.o", "wb")) == NULL) {
		eprintf("Even Helen Keller knows you should be able to write to /tmp, and she's fucking dead.\n");
		exit(1);
	}
}

/*
 * Calibrate the machine and sense timing values, for later
 */
void calibrate() {
	// TODO: time() full power from 0 till a limit signal for X & Y
	X_limit = 10.0; // dummy 
	Y_limit = 10.0; // " " same for Y-axis
	return;
}

/*
 * Prints the actual deliverable, using compiled Gcode generated via Lex
 */
int printOut() {
	DEBUG("Beginning print process. Don protective eyewear. Seriously.");
	fclose(fcomp);
	fcomp = fopen("/tmp/PIC.o","rb");
	rewind(fcomp); // just in case, you know?
	while(!feof(fcomp)) {
		// TODO: fscanf(fcomp,"%" SCNxN "d", 32, &daughter);
		// digitalWrite(1, (daughter & 2));
		break;	
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
    	fprintf(stderr, "Debug: line %d of file %s: ", line, file);
    	vfprintf(stderr, fmt, list);
	fputc('\n', stderr);
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
		

	last_pos_x = pos_x;
	last_pos_y = pos_y;
	last_pos_z = pos_z;
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

/*
 * Gracefully exit, leaving the system in one piece. One.
 */
int cleanup() {
	DEBUG("Finishing up...");
	laseroff();
	fclose(yyin);
	fclose(fcomp);
	// TODO: reset to <0,0,0>
	return 0;
}
