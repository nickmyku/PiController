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

// sanity check at program startup
int sanity(int argc, char **argv);

// calibrates axes
void calibrate();

// runs compiled Gcode, essentially
int printOut();

// called by lexer when axes updated
void axis_updated();

// set axis direction
//void set_dir(int axis, int dir); //moved to drivers/limit_sw

// turns on and off the laser, mcp4451 now takes care of power level
int activateLaser(bool active);

// sets a slave pin high
int setSlaveHigh(FILE *i2c, int pin);

// sets a slave pin low
int setSlaveLow(FILE *i2c, int pin);

// moves the designated axis in the number of steps (signed for direction)
int move(int axis, int steps);

// same as move() but for microstepping
int micromove(int axis, int microsteps);

//add layer of powder
int addLayer(int device_add, int fwd_speed, int rev_speed);

// last thing this program does - turn off pins, free memory, close files/sync
void cleanup();

// Limit switch watchdog thread
void *lim_watchdog(void* ignored);

#endif /* Y_PIC_H_ */

