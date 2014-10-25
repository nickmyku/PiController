/*
Embark Industries
Raspberry Pi I2C Interface for External Microcontroller
External Microcontroller: Arduino
Microcontroller Prog Name: ArduinoLaser
Controls the following
- IR Temp Sensor
- Laser Thermrister
- Relays for Sorensen

Rev3

Created By: Joseph Yosup Shim
Creation Date: 10/25/14

*/
//-------------------------------------------------------------------------------
#ifdef SLAVEMIRCOI2C_MAIN
//special includes
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

#include "../slave/debug.h"
#include "../slave/debug.c"
#include "../slave/commands.h"
//Includes
#include "i2c_helper.c"		//helper functions and definitions for i2c
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "mlx90614.h"
//Defines


int main(void)
{
	int SlaveMicro = 0; //device name
	SlaveMicro = initializeSlaveMicroI2C(); //check to see if device exists and store device name
	
	wiringPiI2CWrite(SlaveMicro,3);
	
}


#endif
//------------------------------------------------------------------------------
//#define MLXADDR 0x5A; //address for the IR sensor

int initializeSlaveMicroI2C(void)
{
	int device; //device name
	
	if((device = wiringPiI2CSetup(0x5A)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	return device;
}

int writeCommandSlaveMicroI2C(int dev, int command)
{
	int value = 0;
	
	wiringPiI2CWrite(dev,command);
	//writeReg(dev,0x5A,0x07); //command
}

float readResultSlaveMicroI2C(void)
{
	int value = 0;

	value = wiringPiI2CRead(dev);//readDoubleReg(dev,0x5A); //Read data from MLX

	double TempData = value * 0.02; // value * 0.02 gives kelvin
	float Celcius = TempData - 273.15; //kelvin - 273.15 gives Celcius

	return Celcius;
}
