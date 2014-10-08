/*
Embark Industries
Raspberry Pi I2C Interface for MLX90614
IR Temp Sensor
Created By: Joseph Yosup Shim
Creation Date: 10/05/14

*/

#ifdef MLX90614_MAIN
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

#include "./slave/debug.h"
#include "./slave/debug.c"

//Includes
#include "i2c_helper.c"		//helper functions and definitions for i2c

//Defines


int main(void)
{
	int MLX; //device name
	MLX = initializeMLX(); //check to see if device exists and store device name

	float temp = IRreadTemp(MLX);
	printf("%f \n", temp);
}


#endif

#define MLXADDR 0x5A; //address for the IR sensor

int initializeMLX(void)
{
	int device; //device name
	
	if((device = wiringPiI2CSetup(MLXADDR)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	return device;
}

float IRreadTemp(int dev)
{
	int value = 0; //i2c return value from the MLX

	writeReg(dev,MLXADDR,0x07); //command

	//Read the raw temp value and convert to something useful
	value = readDoubleReg(dev,MLXADDR); //Read data from MLX

	double TempData = value * 0.02; // value * 0.02 gives kelvin
	float Celcius = TempData - 273.15; //kelvin - 273.15 gives Celcius

	return Celcius;

}