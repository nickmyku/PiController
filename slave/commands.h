/*
 * File: commands.h
 * Author: Anthony Cargile
 * Description: defines op codes for PIC (both slave daughter board and 
 *     control program at once)
 * Copyright: 2014 Embark Industries, all rights reserved. 
 */
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/*
 * ***** DEFINITIONS ****
 */
#define SLAVE_BUS_ID 0x04
#define MASTER_BUS_ID 0x03
#define X_PIN 0
#define Y_PIN 2
#define Z_PIN 4
#define V_PIN 6
#define LASER_PIN 1
#define X_DIR_PIN 10
#define Y_DIR_PIN 3
#define Z_DIR_PIN 5
#define V_DIR_PIN 11
#define LIM_X 17
#define LIM_Y 18
#define LIM_Z 19
#define LIM_V 20

/*
 * ***** COMMANDS *****
 */
// sent when first initializing
#define OP_HELLO 1

// stops the slave's processing
#define OP_HALT 2

// undo's the OP_HALT command
#define OP_UNHALT 3

// triggers a hard-reset
#define OP_RESET 4

// sets a pin to high
// arguments: (int) pin, the Arduino pin
#define OP_SET_HIGH 5

// sets a pin to low
// arguments: (int) pin, the Arduino pin
#define OP_SET_LOW 6

// repeats next command 
// arguments: (int) number of times to repeat
// usually scaled to time determined via calibration
#define OP_REPEAT 7

// move some axis
#define OP_MOVE_X 8
#define OP_MOVE_Y 9
#define OP_MOVE_Z 10

// 11-13 reserved for microstepping

#endif /* _COMMANDS_H_ */
