#ifndef SX1509_C_
//driver for the sx1509 io expander board in the REV 2 3d metal printer prototype
//Nicholas Mykulowycz
//Created April 20, 2014

#ifdef __clang__
__has_feature(cxx_binary_literals)
#endif

#if __GNUC_MINOR__ < 3
#include "bin2hex.h"
#endif

//global varible declarations
const int motorPwmPin = 3;	//pin (on Data Register A) that the pwm signal of the dc motor driver is connected to
const int w_lim_0 = 0;		//pin (on Data Register B) where w limit switch 0 is connected	
const int w_lim_1 = 1;		//pin (on Data Regsiter B) where w limit switch 1 is connected
const int w_timeout = 200000;	//timeout value for w axis "move to limit" function
const int delayMS = 100;	//delay between limit switch checks on w axis "move to limit" function

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

/* Commented out via macro */
#ifdef SX1509_MAIN

/*	following includes need to be in parent file	*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

#include "i2c_helper.c"		//helper functions and definitions for i2c 
#include "../slave/debug.h"	//DEBUG functions
#include "../slave/debug.c"	//DEBUG functions
#include "../slave/commands.h"

#include "sx1509.h"

int main (void)
{
	//create identifier for sx1509 io expander
	int sx;
	
	sx = initializeDevice();
	
	/*
	//establish i2c connection to sx1509
	if((sx = wiringPiI2CSetup(SXADDR)) == -1)
	{
		fprintf(stderr, "sx: unable to initialize I2C %s, activating self destruct sequence\n", strerror (errno));
		DEBUG("error: function \"selfDestruct()\" not defined in scope of program\n");
		DEBUG("alright fine you win this time\n");
		return -1;
	}
	*/
	
	/*
	//calls function which sets input and output pin parameters
	if(initPinMode(sx) != 0)
	{
		DEBUG("error setting up pin modes\n");
		
		return -1;
	}
	*/
	
	//disabled outputs, led driver on
	//writeReg (sx, REGDATAA, 0b00000000);
	//writeReg (sx, REGDATAB, 0b00000000);
	
	//bool localbits[8];
	
	//LEDColor(sx, 0, 1,254,1);
	//int var =121;
	//printf("%d\n",var);
	//intToBits(localbits,var);
	//var = bitsToInt(localbits);
	//printf("%d\n",var);
	
	//motorMove(sx, false, 200);
	//delay(1000);
	//motorStop(sx, false);
	
	//rave(sx, 1000, 76);
	
	LEDBreath(sx, 4, 255, 50);
	LEDBreath(sx, 12, 255, 50);
	
	moveToLimit(sx, false, 200, true);	
	
	DEBUG("W limit 0 depressed - %d\n", readWLimit(sx, 0));
	DEBUG("W limit 1 depressed - %d\n", readWLimit(sx, 1));
	
	DEBUG("program complete\n");

	return 0;
}
#endif /* SX1509_MAIN */

int initializeDevice ()
{
	int device;
	
	//intialize I2C connection to sx1509 
	if((device = wiringPiI2CSetup(SXADDR)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	
	//software reset of device
	writeReg (device, REGRESET, 0x12);
	writeReg (device, REGRESET, 0x34);
	
	//intialize pin modes on sx1509
	writeReg (device, REGDIRA, 0b10000000);		//register A pin modes
	writeReg (device, REGDIRB, 0b10001111);		//register B pin modes
	
	//enables oscillator, led drivers dont work when its disabled
	writeReg (device, REGCLOCK, 0b01000000);
	
	//misc register setup, log fade and oscillation divider
	writeReg (device, REGMISC, 0b11000000);
	
	//enables led driver for pins 4, 5, 6 Also pin 3 - motor speed control
	writeReg (device, REGLEDDRIVERENABLEA, 0b01111000);
	//enabled LED driver for pins 12, 13, 14
	writeReg (device, REGLEDDRIVERENABLEB, 0b01110000);
	
	//disable input buffer on LED/PWM pins
	writeReg (device, REGINPUTDISABLEA, 0x01111000);
	writeReg (device, REGINPUTDISABLEB, 0b01110000);
	
	//enable open drain on LED/PWM pins
	writeReg (device, REGOPENDRAINA, 0b01111000);
	writeReg (device, REGOPENDRAINB, 0b01110000);
	
	//enable pull up resitors on LED/PWM pins
	writeReg (device, REGPULLUPA, 0b01111000);
	writeReg (device, REGPULLUPB, 0b01110000);
	
	//seed random
	srand(time(NULL));
	
	return device;
}

//makes LED fade in and out
int LEDBreath(int device, int pin, int onIntensity, int offIntensity)
{
	int tempReg;
	int tempPin;
	int reg_data;
	bool bits[8];
	
	if(REGTRISE[pin] == 0)
	{
		DEBUG("this pin has been asphyxiated, it cannot breathe.\n");
		return -1;
	}
	
	//make sure intensity vale is within range
	onIntensity = scaleIntensity(onIntensity);
	offIntensity = scaleIntensity(offIntensity);
	
	
	//sets led intensity
	writeReg (device, REGION[pin], onIntensity);
	//sets on time
	writeReg (device, REGTON[pin], 0b00000010);
	//sets off time and intensity
	writeReg (device, REGOFF[pin], 0b00011111);
	//sets fade in time
	writeReg (device, REGTRISE[pin], 0b00001111);
	//sets fade out time
	writeReg (device, REGTFALL[pin], 0b00001111);	
	
	if(pin <8)
	{
		tempReg = REGDATAA;
		tempPin = pin;
	}
	else
	{
		tempReg = REGDATAB;
		tempPin = pin - 8;		//accounts for pin value greater than reg size
	}
	
	
	//read current values at reg
	reg_data = readReg (device, tempReg);
	//convert to bits for manipulation reasons
	intToBits(bits, reg_data);
			
	//set bits associated with led pins low
	bits[tempPin] = false;
	//convert back to int
	reg_data = bitsToInt(bits);
			
	//enable PWM output by setting output low
	writeReg (device, tempReg, reg_data);

	return 0;
}

//sets led to constant color
int LEDColor(int device, int LEDNum, int red, int green, int blue)
{
	int redPin =	4;
	int greenPin =	5;
	int bluePin =	6;
	
	bool bits[8];
	int reg_data;
			
	//assigns proper output pins for a given led strip
	switch(LEDNum)
	{
		case 0:
			redPin =	4;
			greenPin =	5;
			bluePin =	6;
			
			//read current values ar reg a
			reg_data = readReg (device, REGDATAA);
			//convert to bits for manipulation reasons
			intToBits(bits, reg_data);
			
			//set bits associated with led pins low
			bits[redPin] = false;
			bits[greenPin] = false;
			bits[bluePin] = false;
			//convert back to int
			reg_data = bitsToInt(bits);
			
			//enable PWM output by setting output low
			writeReg (device, REGDATAA, reg_data);
			break;
		case 1:
			redPin =	12;
			greenPin =	13;
			bluePin =	14;
			
			//read current values ar reg b
			reg_data = readReg (device, REGDATAB);
			//convert to bits for manuipulation reasons
			intToBits(bits, reg_data);
			
			//set bits associated with led pins low
			bits[redPin-8] = false;
			bits[greenPin-8] = false;
			bits[bluePin-8] = false;
			//convert back to int
			reg_data = bitsToInt(bits);
			
			//enable PWM output by setting output low
			writeReg (device, REGDATAB, reg_data);
			break;
		default:
			DEBUG("there is no LED %d, asshole\n",LEDNum);
			return -1;
	}
	
	//make usre red green and blue intensities are in range
	red = scaleIntensity(red);
	green = scaleIntensity(green);
	blue = scaleIntensity(blue);
	
	//sets the intensitys of the RGB leds
	writeReg (device, REGION[redPin], red);
	writeReg (device, REGION[greenPin], green);
	writeReg (device, REGION[bluePin], blue);
	 
	//sets T_ON to default 0x00 (always on)
	writeReg (device, REGTON[redPin], 0x00);
	writeReg (device, REGTON[greenPin], 0x00);
	writeReg (device, REGTON[bluePin], 0x00);
	
	//sets OFF to defualt 0x00 (always on)
	writeReg (device, REGOFF[redPin], 0x00);
	writeReg (device, REGOFF[greenPin], 0x00);
	writeReg (device, REGOFF[bluePin], 0x00);
	
	//turn off fading if its enabled for that pin
	int pins[3] = {redPin,greenPin,bluePin};
	int i;
	for(i=0; i<3; i++)
	{
		if(REGTRISE[pins[i]] != 0)
		{
			writeReg (device, REGTRISE[pins[i]], 0x00);
		}
		if(REGTFALL[pins[i]] != 0)
		{
			writeReg (device, REGTFALL[pins[i]], 0x00);
		}
	}
	
	//printf("red %d, green %d, blue %d\n", red, green, blue);
	
	return 0;
}


//function is critially important to proper function of driver
//any attempt to remove this function is considered a captial offense and is punishable by death
//- a slow and painful death just to be concise
void rave(int device, int cycles, int delayMS)
{

	int r0;
	int g0;
	int b0;
	
	int r1;
	int g1;
	int b1;
	
	int i;
	
	for(i=0; i<cycles; i++)
	{
		r0 = rand()%256;
		g0 = rand()%256;
		b0 = rand()%256;
		
		LEDColor(device, 0, r0, g0, b0);
		delay(delayMS/2);
		
		r1 = rand()%256;
		g1 = rand()%256;
		b1 = rand()%256;
				
		LEDColor(device, 1, r1, g1, b1);
		delay(delayMS/2);
	}
		
}

bool readWLimit(int device, int switchNum)
{
	int reg_data_b;
	bool bits[8];
	
	//read current values ar reg b
	reg_data_b = readReg (device, REGDATAB);
	intToBits(bits, reg_data_b);
	
	switch(switchNum)
	{
		case 0:
			if(bits[0])
			{
				return false;
			}
			else
			{
				return true;
			}
		case 1:
			if(bits[1])
			{
				return false;
			}
			else
			{
				return true;
			}
		default:
			DEBUG("no limit switch connected to %d\n",switchNum);
			return false;
	}
}

int motorMove(int device, bool dir, int speed)
{
	
	int reg_data_a;
	bool bits[8];
	
	//make sure speed value is in range (<255)
	speed = scaleIntensity(speed);
	//account for inverted PWM driver
	speed = 255 - speed;
	
	//read current values ar reg a
	reg_data_a = readReg (device, REGDATAA);
	intToBits(bits, reg_data_a);
	
	//check to make sure the x and y axis gantry is out of the way
	//unfortunately it is not smart enough to know if it is the neg or pos limit
	if(digitalRead(LIM_X) != LIM_TRIGGER)
	{
		return 1;
	}
	
	//positive
	if(!dir)
	{	
		bits[0] = true;		//standby pin
		bits[1] = true;		//B-IN 1 pin
		bits[2] = false;	//B-IN 2 pin
		bits[motorPwmPin] = false;	//PWM pin, must be set low to enable pwm
	}
	//negative
	else
	{
		bits[0] = true;	//standby pin
		bits[1] = false;	//B-IN 1 pin
		bits[2] = true;		//B-IN 2 pin
		bits[motorPwmPin] = false;	//PWM pin, must be set low to enable pwm
	}
	
	reg_data_a = bitsToInt(bits);
	
	//enable motor and set direction
	writeReg (device, REGDATAA, reg_data_a);
		
	//sets the pwm value
	writeReg (device, REGION[motorPwmPin], speed);
	//sets T_ON to default 0x00 (always on)
	writeReg (device, REGTON[motorPwmPin], 0x00);
	//sets OFF to defualt 0x00 (always on)
	writeReg (device, REGOFF[motorPwmPin], 0x00);
	
	return 0;
}

int motorStop(int device, bool brake)
{
	int reg_data_a;
	bool bits[8];
	
	//read current values at reg a
	reg_data_a = readReg (device, REGDATAA);
	intToBits(bits, reg_data_a);
	
	//if brake is true enable brake functionality
	if(brake)
	{
		bits[0] = true;		//standby pin
		bits[1] = true;		//B-IN 1 pin
		bits[2] = true;		//B-IN 2 pin
		bits[motorPwmPin] = false;	//PWM pin, must be set low to enable pwm
	}
	//otherwise let motor coast to a stop
	else
	{
		bits[0] = true;		//standby pin
		bits[1] = false;	//B-IN 1 pin
		bits[2] = false;	//B-IN 2 pin
		bits[motorPwmPin] = false;	//PWM pin, must be set low to enable pwm
	}
	
	reg_data_a = bitsToInt(bits);
	
	//send data to motor pins
	writeReg (device, REGDATAA, reg_data_a);
	
	//sets the pwm value to off
	writeReg (device, REGION[motorPwmPin], 0xff);
	//sets T_ON to default 0x00 (always on)
	writeReg (device, REGTON[motorPwmPin], 0x00);
	//sets OFF to defualt 0x00 (always on)
	writeReg (device, REGOFF[motorPwmPin], 0x00);	
	
	
	return 0;
}

int moveToLimit(int device, bool dir, int speed, bool brake)
{
	bool limit;
	int time = 0;
	
	//check to see if w axis has already reched limit
	if(dir && readWLimit(device, 1))
	{
		DEBUG("already at positive limit\n");
		return -1;
	}
	if(!dir && readWLimit(device, 0))
	{
		DEBUG("already at negative limit\n");
		return -2;
	}
	
	//if limit is not already reached begin movement
	if(motorMove(device, dir, speed) != 0)
	{
		return -4;
	}
	
	//loop which continuosly checks limit switches until it detects a change or times out
	if(dir)
	{
		while(!limit && (time<w_timeout))
		{
			//check if limit switch is depressed
			limit = readWLimit(device, 1);
			
			//delay to prevert the i2c bus from being flooded
			delay(delayMS);
			time += delayMS;
		}
	}
	else
	{
		while(!limit && (time<w_timeout))
		{
			//check if limit switch is depressed
			limit = readWLimit(device, 0);
			
			//delay to prevent i2c bus from being flooded
			delay(delayMS);
			time += delayMS;
		}
	}
	
	//stop motor once limit is reached
	motorStop(device, brake);
	
	//inform user if timeout condition occured
	if(time>w_timeout)
	{
		printf("movement timed out, motor isnt powered or it got stuck\n");
		return -3;
	}
		
	return 0;
}



#endif /* SX1509_C_ */
