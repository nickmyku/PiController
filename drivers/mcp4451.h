#ifndef MCP4451_H_
//header file of driver for the mcp4451 digital potentiometer chip
//used to control sorensen DCS60-18 supplies
//Nicholas Mykulowycz
//Created May 26, 2014

//prototype functions
int initializeMCP();
/*
	returns -1 if I2C connection can not be established
	
	otherwise returns the id of the device
*/

int setLaserPower(int device, double precent);
/*
	returns -1 if precent value is less than 0 or greater than 100
	
	otherwise returns 1

*/

int setLaserCurrent(int device, double current, bool dual_supply);
/*
	returns 0 if requested current was less than or equal to 0
	
	returns 1 if the requested current was greater than 0 but less than the maximum current
	
	returns -1 if the requested current was greater than max current, 
	also tries to break your will with a sarcastic debug statement insulting your intelligence

int setVoltage(int device, int supply, double voltage);
/*
	returns -1 if you requested a supply that does not exist
	
	otherwise returns 1
*/

int setCurrent(int device, int supply, double current);
/*
	returns -1 if you requested a supply that does not exist
	
	otherwise returns 1
*/

double getVoltage(int device, int supply);
/*
	returns -1 if you requested a supply that does not exist
	
	otherwise returns the 8 bit value corresponding to the voltage
*/

double getCurrent(int device, int supply);
/*
	returns -1 if you requested a supply that does not exist
	
	otherwise returns the 8 bit value corresponding to the voltage
*/

int scaleTo255(double value, double max_value);
/*
	returns the value after it has been converted to an 8 bit number and inverted


*/
double scaleToHuman(int value, double max_value);
/*
	guaranteed to convert those pesky 8 bit values into something that is human readable, or your money back!
	
	returns doubel corresponding to the integer value scaled against the max value
	
*/	
void setPotValue(int device, int pot_addr, int value);
int readPotValue(int device, int pot_addr);
/*
	returns the value of the potentiometer registers as a 16 bit integer value
	ps - its formatted backwards, im sure that will cause problems when someone else takes ownership of te code
	...but thats not my problem
	
	*/

#endif  /* MCP4451_H_ */