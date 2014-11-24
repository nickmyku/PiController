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

//arduino internal register addresses
const int MEGAADDR	= 0x64;
const int THERM		= 0x01;	//address of the thermistor
const int RCLOSE	= 0x04;	//address to set relays to closed
const int ROPEN		= 0x03;	//address to set relays to open
const int RELAY		= 0x09;	//unused address for the relays


//-------------------------------------------------------------------------------
#ifdef SLAVE_MICRO_MAIN
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
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "i2c_helper.c"		//helper functions and definitions for i2c
#include "slave_micro.h"
//Defines


int main(void)
{
	int SlaveMicro = 0; //device name
	SlaveMicro = initializeSlaveMicro(); //check to see if device exists and store device name
	float temp = 0;
	int val = 3;
	
	temp = getLaserTemp(SlaveMicro);
	//wiringPiI2CWrite(SlaveMicro, ROPEN);
	//3 enable power
	//4 disable

	val = enableLaser(SlaveMicro, 0);
	
	DEBUG("\nLaser Temp: %f\n", temp);
	DEBUG("\nrelays state: %d\n",val);
}


#endif
//------------------------------------------------------------------------------


int initializeSlaveMicro(void)
{
	int device; //device name
	
	if((device = wiringPiI2CSetup(MEGAADDR)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	return device;
}

int enableLaser(int dev, bool enabled)
{
	// if embaled is true then send the command to disengague the relays
	// whihc will enable the power supply
	if(enabled){
		//using read reg here is a hack for a write register command
		readReg(dev, ROPEN);
		return 1;
	}
	// otherwise close the relays, setting the power supply to 0
	else{
		//using read reg here is a hack for a write register command
		readReg(dev, RCLOSE);
		return 0;
	}

}

//int writeCommandSlaveMicroI2C(int dev, int command)
//{
	//int value = 0;
	
	//wiringPiI2CWrite(dev,command);
	//writeReg(dev,0x5A,0x07); //command
//}

float getLaserTemp(int dev)
{
	int value = 0;
	
	// kindly ask the arduino to return the temperature value
	value = readReg(dev, THERM);
	
	double TempData = value * 0.02; // value * 0.02 gives kelvin
	float Celcius = TempData - 273.15; //kelvin - 273.15 gives Celcius

	return Celcius;
}
