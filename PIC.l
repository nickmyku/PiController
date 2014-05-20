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
#include "drivers/axis_config.h"
    
#ifndef TEST_BUILD
// building on non-Pi, include dummy GPIO interface:
#include "gpio_dummy.h"
#else
// real deal, include real GPIO library files:
#include <wiringPi.h>
#include <wiringPiI2C.h>
#define pthread_yield_np() pthread_yield(NULL)
#endif

#define COMPILED_FILE "/tmp/PIC.o"
#define V_MAX (double)250.0 /* mm/min */
#define NSEC_DELAY 500000.0 /* Nanoseconds between signals */


typedef struct
{
	int x;
	int y;
	int z;
	int v;
} lockout_t;

// globals (don't hate):
FILE *fcomp; // compiled output (/tmp file)
int i2cfile;
    
// SX1509 Driver:
int sx = -1;
    
// position storage (shared pthread memory)
double pos_x = 0.0;
double pos_y = 0.0;
double pos_z = 0.0;
double pos_v = 0.0;
double l_power = 0.0; // laser intensity, from 0.0 to 100.0
double last_pos_x = 0.0;
double last_pos_y = 0.0;
double last_pos_z = 0.0;
double last_pos_v = 0.0;
double x_steps_per_mm;
double y_steps_per_mm;
double z_steps_per_mm;
double v_steps_per_mm;
int dline = 0;
char x_ready = 0;
char y_ready = 0;
pthread_t lim_thread;
lockout_t lockouts;

%}

%s MOVEMENT
%s X
%s Y
%s Z
%s V
%s L

letter [a-zA-Z]
digit [0-9]
newline \n
whitespace [ \t]+

misc "M"
general "G"
xaxis "X"
yaxis "Y"
zaxis "Z"
vaxis "V"
laxis "L"
point "."
sign "-"
%%

<INITIAL>{general}"0"?"1" {/* G[0]1 */BEGIN MOVEMENT;}
<INITIAL>{misc}"0"?"3"{whitespace}*{newline}+{misc}"0"?"5" ;
<INITIAL>{misc}"0"?"3" {/* M[0]3 */}
<INITIAL>{misc}"0"?"5" {/* M[0]5 */}
<INITIAL>{misc}"1""0" {/* M10 */moveToLimit(sx, false, 200, true);}
<INITIAL>{misc}"1""1" {/* M11 */moveToLimit(sx, true, 200, true);}
<INITIAL>{misc}"0"?"2" {/* M[0]2 */yyterminate();}

<MOVEMENT>{xaxis}{sign}?{digit}+{point}{digit}+ {/* G[0]1 X#.# */yyless(1);BEGIN X;}
<MOVEMENT>{yaxis}{sign}?{digit}+{point}{digit}+ {/* G[0]1 Y#.# */yyless(1);BEGIN Y;}
<MOVEMENT>{zaxis}{sign}?{digit}+{point}{digit}+ {/* G[0]1 Z#.# */yyless(1);BEGIN Z;}
<MOVEMENT>{vaxis}{sign}?{digit}+{point}{digit}+ {/* G[0]1 V#.# */yyless(1);BEGIN V;}
<MOVEMENT>{laxis}{sign}?{digit}+{point}{digit}+ {/* G[0]1 L#.# */yyless(1);BEGIN L;}
<MOVEMENT>{newline} {/* EOL of G[0]1 */dline++;sleep(1);DEBUG("Axis Updated (%d)", dline);axis_updated();BEGIN INITIAL;}
<MOVEMENT><<EOF>> {/* EOF during G[0]1 reading */dline++;axis_updated();DEBUG("EOF and Axis Updated (%d)", dline);yyterminate();}

<X>{sign}?{digit}+{point}{digit}+ {pos_x = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Y>{sign}?{digit}+{point}{digit}+ {pos_y = strtod(yytext, NULL);BEGIN MOVEMENT;}
<Z>{sign}?{digit}+{point}{digit}+ {pos_z = strtod(yytext, NULL);BEGIN MOVEMENT;}
<V>{sign}?{digit}+{point}{digit}+ {pos_v = strtod(yytext, NULL);BEGIN MOVEMENT;}
<L>{digit}+{point}{digit}+ {l_power = strtod(yytext, NULL);BEGIN MOVEMENT;}
<X,Y,Z,V>{whitespace}+ {DEBUG("What, did Charlie Sheen write this shit? (line %d)", dline);BEGIN MOVEMENT;}

<INITIAL,X,Y,Z,V>{newline} {dline++;}
<MOVEMENT,INITIAL>{whitespace}+ ;
<INITIAL,MOVEMENT,X,Y,Z,V>. {DEBUG("I didn't sign up for this: '%s' (line %d)", yytext, dline);}

%%

/*
 * THE entry point - keep it pure, kids. Just like your heroin.
 */
int main(int argc, char **argv) {
	sanity(argc, argv);
	LEDColor(sx, 0, 255, 0, 0);	//set LEDs to red to indicate print in progress
	LEDColor(sx, 1, 255, 0, 0);
	yylex();
        cleanup();
        LEDColor(sx, 0, 0, 255, 0);	//set LEDS to gree to indicate pritning finished
	LEDColor(sx, 1, 0, 255, 0);
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
    pinMode(V_PIN, OUTPUT);
    pinMode(X_DIR_PIN, OUTPUT);
    pinMode(Y_DIR_PIN, OUTPUT);
    pinMode(Z_DIR_PIN, OUTPUT);
    pinMode(V_DIR_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(LIM_X, INPUT);
    pinMode(LIM_Y, INPUT);
    pinMode(LIM_Z, INPUT);
    
    // set up axis and motor parameters
    x_steps_per_mm = stepsPerMM('x'); 	
    y_steps_per_mm = stepsPerMM('y');
    z_steps_per_mm = stepsPerMM('z');
    v_steps_per_mm = stepsPerMM('v');
    
    //reset lockouts
    lockouts.x = LOCKOUT_NULL;
    lockouts.y = LOCKOUT_NULL;
    lockouts.z = LOCKOUT_NULL;
    lockouts.v = LOCKOUT_NULL;
    
    // set up driver(s):
    if((sx = initializeDevice()) == -1) {
        eprintf("\"Fatal issue involving the driver\"\n-Princess Diana");
        pthread_exit(NULL);
    }
    
    // set up watchdog thread (limit switch detection)
    lockouts.x = lockouts.y = lockouts.z = lockouts.v = LOCKOUT_NULL;
    int rc = pthread_create(&lim_thread, NULL, lim_watchdog, (void *)NULL);
    if (rc) {
        eprintf("Could not allocate a thread for the Y-axis.\nProposed resolution: swallow cyanide.");
        pthread_exit(NULL);
    }
}

/*
 * Watch over limit switches and lock out an axis direction if tripped
 */
void *lim_watchdog(void* ignored) {
    struct timespec watchdelay;
    watchdelay.tv_nsec = 50000; // tweak as needed
    
    while(1) {
        nanosleep(&watchdelay,NULL);
        
	    //check if X lim is triggered and that the lockout has not already been set
	    if(digitalRead(LIM_X) == LIM_TRIGGER && lockouts.x == LOCKOUT_NULL) {
		//if last move was in negative direction set the negative lockout
		if(digitalRead(X_DIR_PIN) == DIR_NEG) {
		   lockouts.x = LOCKOUT_NEG;
		}
		//if last move was in positive direction set the positive lockout
		if(digitalRead(X_DIR_PIN) == DIR_POS) {
		   lockouts.x = LOCKOUT_POS;
		}
	    }
	    //check if Y lim is triggered and that the lockout has not already been set
	    if(digitalRead(LIM_Y) == LIM_TRIGGER && lockouts.y == LOCKOUT_NULL) {
		//if last move was in negative direction set the negative lockout
		if(digitalRead(Y_DIR_PIN) == DIR_NEG) {
		   lockouts.y = LOCKOUT_NEG;
		}
		//if last move was in positive direction set the positive lockout 
		if(digitalRead(Y_DIR_PIN) == DIR_POS) {
		   lockouts.y = LOCKOUT_POS;
		}		
	    }
	    //check if Z lim is triggered and that the lockout has not already been set
	    if(digitalRead(LIM_Z) == LIM_TRIGGER && lockouts.z == LOCKOUT_NULL) {
		//if last move was in negative direction set the negative lockout
		if(digitalRead(Z_DIR_PIN) == DIR_NEG) {
		   lockouts.z = LOCKOUT_NEG;
		}
		//if last move was in positive direction set the positive lockout 
		if(digitalRead(Z_DIR_PIN) == DIR_POS) {
		   lockouts.z = LOCKOUT_POS;
		}		
	    }
	    //check if V lim is triggered and that the lockout has not already been set
	    if(digitalRead(LIM_V) == LIM_TRIGGER && lockouts.v == LOCKOUT_NULL) {
		//if last move was in negative direction set the negative lockout
		if(digitalRead(V_DIR_PIN) == DIR_NEG) {
		   lockouts.v = LOCKOUT_NEG;
		}
		//if last move was in positive direction set the positive lockout 
		if(digitalRead(V_DIR_PIN) == DIR_POS) {
		   lockouts.v = LOCKOUT_POS;
		}		
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
double steps_z;
double steps_v;
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
    double dv;
    double dz;
    double x_vel;
    double y_vel;
    double temp_x_time;
    double temp_y_time;
    double nano_x_time;
    double nano_y_time;
    double i;

    DEBUG("Turning laser %s", (l_power > 0.0)?("on"):("off"));
    laser(l_power);
    
    DEBUG("\nFrom: <%f,%f,%f> to <%f,%f,%f>",last_pos_x, last_pos_y, last_pos_z, pos_x, pos_y, pos_z);
    
    dx = pos_x - last_pos_x;
    dy = pos_y - last_pos_y;
    dz = pos_z - last_pos_z;
    dv = pos_v - last_pos_v;
    last_pos_x = pos_x;
    last_pos_y = pos_y;
    last_pos_z = pos_z;
    last_pos_v = pos_v;
    pos_x = pos_y = pos_z = pos_v = 0.0; // changed from -1.0 by AC@5/11/14
    x_writes = y_writes = 0.0;
    
    steps_x = dx*x_steps_per_mm;
    steps_y = dy*y_steps_per_mm;
    steps_z = stepsPerMM('Z') * dz;
    steps_v = stepsPerMM('V') * dv;
    
    DEBUG("dX: %f, dY: %f, dZ: %f, dV: %f",dx,dy,dz,dv);

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
	set_dir(X_DIR_PIN, DIR_POS);
    }
    else {
	set_dir(X_DIR_PIN, DIR_NEG);
    }
    if(dy >= 0.0) {
	set_dir(Y_DIR_PIN, DIR_POS);
    }
    else {
	set_dir(Y_DIR_PIN, DIR_NEG);
    }

    //WILL NEED TO FIND MORE INTELLIGENT WAY TO PREVENT MOTOR FROM MOVING 
    //IN THE LOCKOUT DIRECTION, FOR NOW IT WILL JUST STOP MOVING IN EITHER DIRECTION

    // Z Axis
    if(steps_z != 0.0 && lockouts.z == LOCKOUT_NULL) {
	if(dz >= 0.0) digitalWrite(Z_DIR_PIN, 1);
	else digitalWrite(Z_DIR_PIN, 0);
	for(i = 0.0; i <= fabs(steps_z); i++) {
	    digitalWrite(Z_PIN, 1);
            nanosleep(&constdelay,NULL);
            digitalWrite(Z_PIN, 0);
            nanosleep(&constdelay,NULL);
	}
    }

    // V Axis 
    if(steps_v != 0.0 && lockouts.v == LOCKOUT_NULL) {
	if(dv >= 0.0) digitalWrite(V_DIR_PIN, 1);
	else digitalWrite(V_DIR_PIN, 0);
	for(i = 0.0; i <= fabs(steps_v); i++) {
	    digitalWrite(V_PIN, 1);
            nanosleep(&constdelay,NULL);
            digitalWrite(V_PIN, 0);
            nanosleep(&constdelay,NULL);
	}
    }
    
    // Y Axis (threaded and gluten-free!)
    if(steps_y != 0.0 && lockouts.y == LOCKOUT_NULL) {
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
    
    // X Axis
    if(steps_x != 0.0 && lockouts.x == LOCKOUT_NULL) {
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

    DEBUG("No Mr. Bond, I expect you to (NOT) die!");
    l_power = 0.0;
    laser(l_power); // last one out, get the lights
}

#pragma line 452
/*
 * Sets the direction of an axis, adhering to the limit switch lockouts
 */
void set_dir(int axis, int dir) {
	switch(axis) {
		case X_DIR_PIN:
			if(lockouts.x != (dir + 1)) { 
				digitalWrite(X_DIR_PIN, dir);
				//only reset lockout if sensor is no longer triggered, fixes small reverse movement bug
				if(lockouts.x != LOCKOUT_NULL && digitalRead(LIM_X) != LIM_TRIGGER) { 
					lockouts.x = LOCKOUT_NULL;
				}
			}
			break;
		case Y_DIR_PIN:
			if(lockouts.y != (dir + 1)) { 
				digitalWrite(Y_DIR_PIN, dir);
				//only reset lockout if sensor is no longer triggered, fixes small reverse movement bug
				if(lockouts.y != LOCKOUT_NULL && digitalRead(LIM_Y) != LIM_TRIGGER) {
					lockouts.y = LOCKOUT_NULL;
				}
			}
			break;
		case Z_DIR_PIN:
			if(lockouts.z != (dir + 1)) { 
				digitalWrite(Z_DIR_PIN, dir);
				//only reset lockout if sensor is no longer triggered, fixes small reverse movement bug
				if(lockouts.z != LOCKOUT_NULL && digitalRead(LIM_Z) != LIM_TRIGGER) {
					lockouts.z = LOCKOUT_NULL;
				}
			}
			break;
		case V_DIR_PIN:
			if(lockouts.v != (dir + 1)) { 
				digitalWrite(V_DIR_PIN, dir);
				//only reset lockout if sensor is no longer triggered, fixes small reverse movement bug
				if(lockouts.v != LOCKOUT_NULL && digitalRead(LIM_V) != LIM_TRIGGER) {
					lockouts.v = LOCKOUT_NULL;
				}
			}
			break;
		case LASER_PIN:
			eprintf("I've got a mouse and he hasn't got a house\nI don't know why I call him Gerald.\nHe's getting rather old, but he's a good mouse.");
			break;
		default:
			eprintf("Operator chromosome limit exceeded\nTrying to set a direction for axis '%d'\n\nDumbass.", axis);
			break;
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

	if(power == 100.0) {
		digitalWrite(LASER_PIN, 1);
	}
	else if(power == 0.0) {
		digitalWrite(LASER_PIN, 0);
	}
	else {
		DEBUG("What we have here is a laser incapable of PWM control");
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
    digitalWrite(V_PIN,0);
    digitalWrite(X_DIR_PIN,0);
    digitalWrite(Y_DIR_PIN,0);
    digitalWrite(Z_DIR_PIN,0);
    digitalWrite(V_DIR_PIN,0);
    digitalWrite(LASER_PIN,0);
    fclose(fcomp);
    system("sync");
}
