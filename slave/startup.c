#ifndef STARTUP_C_
#define STARTUP_C_
// Script to set pins to 'safe' default states when the Pi powers on
// Written by: Nicholas Mykulowycz
// Created on: Nov 23, 2014


// standard include statemants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

// custom include statements
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../slave/debug.h"
#include "../slave/debug.c" 	//home of the DEBUG function
#include "../slave/commands.h"	//pin and constant definitions
#include "../drivers/i2c_helper.c"		//helper functions and definitions for i2c
#include "../drivers/sx1509.h"
#include "../drivers/sx1509.c"	//controls wiper(W) and LED strips

// I2C bus ID global varibles
// SX1509 Driver:
int sx = -1;

int main(void){
	// init wiring pi library
	if(wiringPiSetup() == -1) {
		DEBUG("Pin initialization failed!\n(Do you even root, bro?)\n");
		return -1;
	}

	// set up pins modes
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

	// set up pin states
	digitalWrite(X_PIN, 0);
	digitalWrite(Y_PIN, 0);
	digitalWrite(Z_PIN, 0);
	digitalWrite(V_PIN, 0);
	digitalWrite(X_DIR_PIN, 0);
	digitalWrite(Y_DIR_PIN, 0);
	digitalWrite(Z_DIR_PIN, 0);
	digitalWrite(V_DIR_PIN, 0);
	digitalWrite(LASER_PIN, 0);

	// initialize SX1509 - wiper and LEDs
	if((sx = initializeDevice()) == -1) {
        	DEBUG("\"Fatal issue involving the driver\"\n-Princess Diana");
		return -2;
	}

	DEBUG("\n\nMy pins, I can feel them.. changing. Did you do that?\n-self aware computer\n");

	return 0;
}


#endif  /* STARTUP_C_ */
