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
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "mlx90614.h"
//Defines

//sx1509 internal register addresses
const int SXADDR = 0x3e;
const int REGRESET =  0x7d;
const int REGPOLARITYA = 0x0d;
const int REGPOLARITYB = 0x0c;
const int REGPULLUPA = 0x07;
const int REGPULLUPB = 0x06;
const int REGDIRA =	0x0f;
const int REGDIRB =	0x0e;
const int REGINPUTDISABLEA = 0x01;
const int REGINPUTDISABLEB = 0x00;
const int REGOPENDRAINA = 0x0b;
const int REGOPENDRAINB = 0x0a;
const int REGDATAA = 0x11;
const int REGDATAB = 0x10;
const int REGMISC = 0x1f;
const int REGCLOCK = 0x1e;
const int REGLEDDRIVERENABLEA =	0x21;
const int REGLEDDRIVERENABLEB = 0x20;
int REGION[16] = {0x2a,0x2d,0x30,0x33,0x36,0x3b,0x40,0x45,0x4a,0x4d,0x50,0x53,0x56,0x5b,0x60,0x65};
int REGTON[16] = {0x29,0x2c,0x2f,0x32,0x35,0x3a,0x3f,0x44,0x49,0x4c,0x4f,0x52,0x55,0x5a,0x5f,0x64};
int REGOFF[16] =  {0x2b,0x2e,0x31,0x34,0x37,0x3c,0x41,0x46,0x4b,0x4e,0x51,0x54,0x57,0x5c,0x61,0x66};
int REGTRISE[16] =	{   0,   0,   0,   0,0x38,0x3d,0x42,0x47,   0,   0,   0,   0,0x58,0x5d,0x62,0x67};
int REGTFALL[16] =	{   0,   0,   0,   0,0x39,0x3e,0x43,0x48,   0,   0,   0,   0,0x59,0x5e,0x63,0x68};


int main(void)
{
	int MLX; //device name
	MLX = initializeMLX(); //check to see if device exists and store device name

	float temp = IRreadTemp(MLX);
	printf("%f \n", temp);
}


#endif

//#define MLXADDR 0x5A; //address for the IR sensor

int initializeMLX(void)
{
	int device; //device name
	
	if((device = wiringPiI2CSetup(0x5A)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	return device;
}

float IRreadTemp(int dev)
{
	int value = 0; //i2c return value from the MLX

	writeReg(dev,0x5A,0x07); //command

	//Read the raw temp value and convert to something useful
	value = readDoubleReg(dev,0x5A); //Read data from MLX

	double TempData = value * 0.02; // value * 0.02 gives kelvin
	float Celcius = TempData - 273.15; //kelvin - 273.15 gives Celcius

	return Celcius;

}
