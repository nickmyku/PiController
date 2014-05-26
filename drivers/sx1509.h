#ifndef SX1509_H_
//header file for sx1509 driver
//created by Nicholas Mykuylowycz
//created on April 23, 2014

//prototpye function declarations
//int intToBits(bool * array, int number);	//moved to i2c helper
//int bitsToInt(bool array[8]);		//moved to i2c helper
//int scaleIntensity(int num);		//moved to i2c helper
int initializeDevice();
int LEDBreath(int device, int pin, int onIntensity, int offIntensity);
int LEDColor(int device, int LEDNum, int red, int green, int blue);
int motorMove(int device, bool dir, int speed);
int motorStop(int device, bool brake);
int moveToLimit(int device, bool dir, int speed, bool brake);
bool readWLimit(int device, int switchNum);
void rave(int device, int cycles, int delayMS);

int initializeDevice();
/*
	intilization function, must be called before any other sx1509 related command

		returns the device address if initialization completed successfully
		device address must be used in all sx1509 related commands

		returns -1 if an I2C connection could not be established\
*/


int LEDBreath(int device, int pin, int onIntensity, int offIntensity);
/*
	function causes LEDs to fade in and out slowly

		returns 0 if function completed successfully

		returns -1 and terminates function if an non fading pin address is input

	device designates the address of the sx1509
	the address is the output of the initialization function

	pin designates the location of the LED you which to address
	fade functionality is only avalible on pins 4,5,6,7 and pins 12,13,14,15 of sx1509
	at time of writing the pins are connected as follows:

		pin 4 - LED red0
		pin 5 - LED green0
		pin 6 - LED blue0
		pin 7 -NC

		pin 12 - LED red1
		pin 13 - LED green1
		pin 14 - LED blue1
		pin 15 - NC

	onIntensity sets the max brightness of the LED,
	it is a value between 0 and 255
		255 being the brightest value

offIntensity functionality has not been implemented yet
*/


int LEDColor(int device, int LEDNum, int red, int green, int blue);
/*
	function allows the static color of each LED strip to be set

		retuns 0 if function completes successfully

		returns -1 if a LED strip other than 0 or 1 is input and terminates the function

	device designates the address of the sx1509
	the address is the output of the initialization function

	LEDNum designates the led strip to be addressed
	currently only LED strip 0 and LED strip 1 are connected (0 or 1)

	red,green, and blue allow the intensity of each individual color to be set
	they can be any value between 0 and 255
		255 being the brightest value
		for example if red=255,green=0,blue=0 the LED strip wold glow bright red
*/


bool readWLimit(int device, int switchNum);
/*
	function reads the value of the limit switches
	
		returns true if the limit switch has been triggered
		
		returns false of the limit switch HAS NOT been triggered
		also returns false if limit switch does not exist
	
	device designates the address of the sx1509
	the address is the output of the initialization function
	
	switchNum is the number of th switch you wish to check
		0 is the negative limit switch
		
		1 is the positive limit switch
		
		if any other number is input the function will return false
		and tell you the switch is not connected

*/

int moveToLimit(int device, bool dir, int speed, bool brake);
/*
	function moves W axis to positive or negative limit
	the execuation of the function is blocking

		returns 0 if function completes successfully

		returns -1 if user tells the driver to move to positive limit
		but the w axis is already at the positive limit

		returns -2 if the user tells the drive to move to the negative limit
		but the w axis is already at the negative limit

		returns -3 if function times out before limit switch is reached
		
		returns -4 if motor move command could not execute (because
		x axis was not at the positive limti)
		

	device designates the address of the sx1509
	the address is the output of the initialization function

	dir designates the intended direction of movement
		true = postive direction
		false = negative direction

	speed designations the velocity to execute the movement at
	any value between 0 and 255 can be used
		255 is the higest speed
		i have found using any value below 50 seems to stall the motor

	the value of brake determines whether the motor will coast to a stop
	or it will electronically brake
		true = electronic brake
		false = coast to stop
*/


int motorMove(int device, bool dir, int speed);
/*
	function moves motor in a given direction
	(this is a "dumb" movement which will not hault if a limit switch is depressed
	
		returns 1 if motor could not be moved (x axis was not at positive limit)
		
		returns 0 in all other situations
	
	device designates the address of the sx1509
	the address is the output of the initialization function
	
	dir designates the intended direction of movement
		true = postive direction
		false = negative direction
		
	speed designations the velocity to execute the movement at
	any value between 0 and 255 can be used
		255 is the higest speed
		i have found using any value below 50 seems to stall the motor
*/

int motorStop(int device, bool brake);
/*
	function which haults motor movement
	
		returns 0 in all situations	
	
	device designates the address of the sx1509
	the address is the output of the initialization function
	
	the value of brake determines whether the motor will coast to a stop
	or it will electronically brake
		true = electronic brake
		false = coast to stop 
*/

void rave(int device, int cycles, int delayMS);
/*
	this is a blocking function which randomly changes the colors of the LED strips
	this function contains code which is vital to the proper operation of the program
	you should not attempt to remove it from the code
	dont even try to understand the very very complicated code behind its operation
	any attempt disect/remove this code could result in irreversible brain damage
	
	device designates the address of the sx1509
	the address is the output of the initialization function
	
	cycles designates how many color change loops to perform
	
	delayMS designates the approximate millisecond delay between color change events
*/


#endif /* SX1509_H_ */
