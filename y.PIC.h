/*
 * Name          : y.PIC.h
 * Created on    : Feb 9, 2014
 * Author        : Anthony Cargile
 * Description   : Prototypes for the main PIC program
 * Copyright     : 2014 Embark Industries, all rights reserved. 
 */

#ifndef Y_PIC_H_
#define Y_PIC_H_

#define laseron()  laser(100.0)
#define laseroff() laser(0.0)
#define ON_A_SHARK_HEAD 0
#define AXIS_X = 0
#define AXIS_Y = 1
#define AXIS_Z = 2

// use this to provide dev-only feedback. See printDebug() too.
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)

// sanity check at program startup
int sanity(int argc, char **argv);

// calibrates axes
void calibrate();

// runs compiled Gcode, essentially
int printOut();

// don't call this - call DEBUG() instead
void printDebug(const char *fmt, const int line, const char *file, ...);

// error print function, works like printf()
void eprintf(const char *fmt, ...);

// called by lexer when axes updated
void axis_updated();

// set axis direction
void set_dir(int axis, int dir);

// runs the laser at power level
int laser(double power);

// sets a slave pin high
int setSlaveHigh(FILE *i2c, int pin);

// sets a slave pin low
int setSlaveLow(FILE *i2c, int pin);

// moves the designated axis in the number of steps (signed for direction)
int move(int axis, int steps);

// same as move() but for microstepping
int micromove(int axis, int microsteps);

// last thing this program does - turn off pins, free memory, close files/sync
void cleanup();

// Limit switch watchdog thread
void *lim_watchdog(void* ignored);

#endif /* Y_PIC_H_ */

