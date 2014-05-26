#ifndef LIMIT_SW_C_
#define LIMIT_SW_C_
//file for limit switch functions
//Nicholas Mykulowycz
//Created May 26, 2014

#include <stdio.h>
#include <wiringPi.h>

#include "slave/commands.h"
#include "../y.PIC.h" // DEBUG()

typedef struct
{
	int x;
	int y;
	int z;
	int v;
} lockout_t;

lockout_t lockouts;

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

#endif /* LIMIT_SW_C_ */