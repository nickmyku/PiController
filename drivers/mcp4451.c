#ifndef MCP4451_C_
//driver for the mcp4451 digital potentiometer chip
//used to control sorensen DCS60-18 supplies
//Nicholas Mykulowycz
//Created May 26, 2014

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "i2c_helper.c"		//helper functions and definitions for i2c

/*	following includes need to be in parent file	*/ 
//include "../y.PIC.h" // DEBUG()
//include "../slave/commands.h"
//include <wiringPi>
//include <wiringPiI2C>

//int intToBits(bool * array, int number);	//moved to i2c helper
//int bitsToInt(bool array[8]);	

//mcp4451 internal register address
const int MCPADDR_READ	= 0x59;		//address' assume A0 and A1 are set to 0
const int MCPADDR_WRITE	= 0x58;
const int TCON0		= 0x04;
const int TCON1		= 0x0A;
const int POT0		= 0x00;
const int POT1		= 0x01;
const int POT2		= 0x06;
const int POT3		= 0x07;

#ifdef MCP_DRIVER_MAIN
int main (void)
{
	//create identifier for digital potentiometer
	int mcp;
	
	mcp = intializeMCP();

}
#endif	/* MCP_DRIVER_MAIN */

int intitializeMCP ()
{
	int device;
	
	if((device = wiringPiI2CSetup(MCPADDR_READ)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	
	return device;
}

#endif  /* MCP4451_C_ */