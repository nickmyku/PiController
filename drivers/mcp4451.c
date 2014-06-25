#ifndef MCP4451_C_
#define MCP4451_C_
//driver for the mcp4451 digital potentiometer chip
//used to control sorensen DCS60-18 supplies - they will power the laser
//Nicholas Mykulowycz
//Created May 26, 2014


/*	following includes need to be in parent file	*/ 
//#include <stdio.h>
//#include <stdbool.h>
//#include <math.h>
//#include "../slave/debug.h"	//DEBUG function
//#include "../slave/debug.c"	//DEBUG function
//#include "../slave/commands.h"
//#include "i2c_helper.c"	//helper functions and definitions for i2c
//#include <wiringPi.h>
//#include <wiringPiI2C.h>
//#include "mcp4451.h"


//mcp4451 internal register address
const int MCPADDR	= 0x2f;		//address' assume A0 and A1 are set to 1
const int TCON0		= 0x04;
const int TCON1		= 0x0A;
const int POT0		= 0x00;
const int POT1		= 0x01;
const int POT2		= 0x06;
const int POT3		= 0x07;
#define MAX_VOLT	(double)60.0	//maximum voltage setting on supply (DCS60-18E)
#define MAX_CURR	(double)18.0	//maximum current setty on supply (DCS60-18E)
#define MAX_LASER_CURR	(double)20.0	//max current for 15W oclaro MEA-200
#define LASER_VOLT	(double)5.0	//kind of arbitrary, laser diode only ever reaches ~3V

#ifdef MCP4451_MAIN
int main (void)
{
	//create identifier for digital potentiometer
	int mcp;
	int tmp;
	double volt0;
	double volt1;
	double curr0;
	double curr1;
	
	mcp = intitializeMCP();
	
	//setVoltage(mcp, 0, 0);
	//setVoltage(mcp, 1, 0);
	//setCurrent(mcp, 0, 0.0);
	//setCurrent(mcp, 1, 0.0);
	
	//setLaserCurrent(mcp, 21, true);
	
	//setPotValue(mcp, TCON0, 255);
	
	//setLaserPower(mcp, 0);
	
	volt0 = getVoltage(mcp, 0);
	volt1 = getVoltage(mcp, 1);
	curr0 = getCurrent(mcp, 0);
	curr1 = getCurrent(mcp, 1);
	DEBUG("---Supply 0---\nVoltage: %f\nCurrent: %f\n\n", volt0, curr0);
	DEBUG("---Supply 1---\nVoltage: %f\nCurrent: %f\n\n", volt1, curr1);
	
	
	
	
	
	

}
#endif	/* MCP4451_MAIN */

int intitializeMCP()
{
	int device;
	
	if((device = wiringPiI2CSetup(MCPADDR)) == -1)
	{
		// error out, caller must handle errors:
		return -1;
	}
	//enable all potentiometers, if any pins are tri stated power supplies go to max output
	setPotValue(device, TCON0, 255);
	setPotValue(device, TCON1, 255);
	
	
	return device;
}

int setLaserPower(int device, double precent){
	double current;
	
	if(precent >= 0 && precent <= 100){
		//scale precet to a decimal between 0 and 1
		precent /= 100;
		//multiply max laser current by precentage of power
		current = MAX_LASER_CURR * precent;
		//send command
		setLaserCurrent(device, current, true);
		return 1;
	}
	else{
		DEBUG("how about next time you try selecting a laser power thats actually in range\n");
		return -1;
	}


}

//set the output current of the supplies for the laser
int setLaserCurrent(int device, double current, bool dual_supply){
	
	if(current <= 0){
		if(dual_supply){
			//set voltages to 0
			setVoltage(device, 0 ,0);
			setVoltage(device, 1, 0);
			//set currens to 0
			setCurrent(device, 0, 0);
			setCurrent(device, 1, 0);
		}
		else{
			//set voltage to 0
			setVoltage(device, 0, 0);
			//set current to 0
			setCurrent(device, 0 ,0);
		}
		//short out laser pins - power supplies dont actually go to 0
		digitalWrite(LASER_PIN, 0);
		return 0;
	}
	//as long as requested current is below mx send commands
	else if(current <= MAX_LASER_CURR){
		//if both supplies are in use divide current by 2 and send commands
		if(dual_supply){
			setCurrent(device, 0, (current/2));
			setCurrent(device, 1, (current/2));
			setVoltage(device, 0, LASER_VOLT);
			setVoltage(device, 1, LASER_VOLT);
		}
		//otherwise just send current command
		else{
			setCurrent(device, 0, current);
		}
		return 1;
	}
	//if requested laser current is greater than software defined max, throw error
	else{
		DEBUG("does %f sound 'less than or equal to' %f to you?\n", current, MAX_LASER_CURR);
		return -1;
	}
		



}

//sets the output voltage of sorensen supply 0 or 1
int setVoltage(int device, int supply, double voltage){
	int pot_reg;
	int voltage_255;
	
	switch(supply)
	{
		case 0:
			pot_reg = POT0;
			break;
		case 1:
			pot_reg = POT3;
			break;
		default:
			DEBUG("you humans are always asking the world of me, that power supply doesnt even exist\n");
			return -1;
	}
	
	voltage_255 = scaleTo255(voltage, MAX_VOLT);
	
	//255 is the 0 voltage value, set pot function interperates 256 as 511
	//and further lowers voltage
	if(voltage_255 ==255){
		voltage_255 = 256;
	}
	
	setPotValue(device, pot_reg, voltage_255);


}

//sets the output current of soresen supply 0 or 1
int setCurrent(int device, int supply, double current){
	int pot_reg;
	int current_255;
	
	switch(supply)
	{
		case 0:
			pot_reg = POT2;
			break;
		case 1:
			pot_reg = POT1;
			break;
		default:
			DEBUG("you humans are always asking the world of me, that power supply doesnt even exist\n");
			return -1;
	}
	
	current_255 = scaleTo255(current, MAX_CURR);
	
	//255 is the 0 current value, set pot function interperates 256 as 511
	//and further lowers current
	if(current_255 ==255){
		current_255 = 256;
	}
	
	
	setPotValue(device, pot_reg, current_255);
	


}

//takes a double and scales it to a value from 0 to 255
int scaleTo255(double value, double max_value){
	int value_255;
	double temp;
	
	//if desired value is greater than max value then return max value
	if(value > max_value){
		value_255 = 0;		//scale is inverted so 0 is max value
		DEBUG("you want %f?!?! I suppose you will never be happy with %f, this relationship is over\n", value, max_value);
	}
	else if(value < 0){
		value_255 = 255;
		DEBUG("user asked for negative value, is user drunk(Y/n)?\n");
	}
	else{
		temp = (value/max_value)*255;
		value_255 = 255 - (int)temp;
	}
	
	return value_255;
	

}

//
double getVoltage(int device, int supply){
	int pot_reg;
	int voltage_16;
	int voltage_255;
	double voltage;
	bool little_bits[8];
	bool bits[16];
	
	
	switch(supply)
	{
		case 0:
			pot_reg = POT0;
			break;
		case 1:
			pot_reg = POT3;
			break;
		default:
			DEBUG("you humans are always asking the world of me, that power supply doesnt even exist\n");
			return -1;
	}
	
	//read the value of the potentiometer register
	voltage_16 = readPotValue(device, pot_reg);
	
	//convert integer into boolean array for easy manipulation
	intToBits16(bits, voltage_16);
	
	//now take that boolean array and make it into a more sane boolean array
	//...if only SOMEONE didnt use a 10-bit register on an 8-bit device.. looking at you Microchip
	little_bits[7] = bits[15];
	little_bits[6] = bits[14];
	little_bits[5] = bits[13];
	little_bits[4] = bits[12];
	little_bits[3] = bits[11];
	little_bits[2] = bits[10];
	little_bits[1] = bits[9];
	little_bits[0] = bits[8];
	
	//convert sane boolean array back to integer
	voltage_255 = bitsToInt(little_bits);
	
	voltage = scaleToHuman(voltage_255, MAX_VOLT);
	
	return voltage;



}

//
double getCurrent(int device, int supply){
	int pot_reg;
	int current_16;
	int current_255;
	double current;
	bool little_bits[8];
	bool bits[16];
	
	switch(supply)
	{
		case 0:
			pot_reg = POT2;
			break;
		case 1:
			pot_reg = POT1;
			break;
		default:
			DEBUG("you humans are always asking the world of me, that power supply doesnt even exist\n");
			return -1;
	}

	//read the value of the potentiometer register
	current_16 = readPotValue(device, pot_reg);
	
	//convert integer into boolean array for easy manipulation
	intToBits16(bits, current_16);
	
	//now take that boolean array and make it into a more sane boolean array
	//...if only SOMEONE didnt use a 10-bit register on an 8-bit device.. looking at you Microchip
	little_bits[7] = bits[15];
	little_bits[6] = bits[14];
	little_bits[5] = bits[13];
	little_bits[4] = bits[12];
	little_bits[3] = bits[11];
	little_bits[2] = bits[10];
	little_bits[1] = bits[9];
	little_bits[0] = bits[8];
	
	//convert sane boolean array back to integer
	current_255 = bitsToInt(little_bits);
	
	current = scaleToHuman(current_255, MAX_CURR);
	
	return current;
	

}

//
double scaleToHuman(int value, double max_value){
	double human_readable_number;
	int temp;
	
	if(value < 0){
		value = 0;
	}
	if(value > 255){
		value = 255;
	}
	
	temp = 255-value;
	
	human_readable_number = ((double)temp/255)*max_value;
	
	return human_readable_number;
	
}

//dont forget the write to device address is different than the read address
void setPotValue(int device, int pot_addr, int value)
{
	bool addr_bits[8];
	
	//convert address into boolean array
	intToBits(addr_bits, pot_addr);
	
	//rearrage address
	addr_bits[7] = addr_bits[3];
	addr_bits[6] = addr_bits[2];
	addr_bits[5] = addr_bits[1];
	addr_bits[4] = addr_bits[0];
	
	//add the write command bits
	addr_bits[3] = 0;
	addr_bits[2] = 0;
	
	//dont care about this bit, setting to 0
	addr_bits[1] = 0;
	
	//this is technically a data bit but its out of our working range
	//creates short circuit when bit is set to 1
	if(value > 255){
		addr_bits[0] = 1;
		
	}
	else{
		addr_bits[0] = 0;
	}
	
	//convert bits back to integer
	pot_addr = bitsToInt(addr_bits);
	
	//make sure value is not out of range
	value = scaleIntensity(value);
	
	//and finally we can write the value to the potentiometer
	writeReg(device, pot_addr, value);
	
}

//dont forget the read device address is different than the write address
int readPotValue(int device, int pot_addr)
{
	int value = 0;
	bool addr_bits[8];
	
	//convert address into boolean array
	intToBits(addr_bits, pot_addr);
	
	//rearrage address
	addr_bits[7] = addr_bits[3];
	addr_bits[6] = addr_bits[2];
	addr_bits[5] = addr_bits[1];
	addr_bits[4] = addr_bits[0];
	
	//add the write command bits
	addr_bits[3] = 1;
	addr_bits[2] = 1;
	
	//dont care about this bit, setting to 0
	addr_bits[1] = 0;
	
	//this is technically a data bit but its out of our working range
	addr_bits[0] = 0;
	
	//convert bits back to integer
	pot_addr = bitsToInt(addr_bits);
	
	value = readDoubleReg(device, pot_addr);
	
	return value;
}



#endif  /* MCP4451_C_ */