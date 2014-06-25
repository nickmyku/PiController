#ifndef AXIS_CONFIG_C_
#define AXIS_CONFIG_C_
//file which sets up motor and axis parameters
//Nicholas Mykulowycz
//Created May 1, 2014

#include <stdio.h>
#include <string.h>

#define SIZE 4

//--------------------------
//       X - AXIS
//--------------------------
const double x_steps_per_rev	= 400;	//number of steps needed for motor to complete a full rotation
					//standard value is 200
const double x_microsteps	= 16;	//number of microsteps per step
					//if microstepping is not in use default value is 1
const double x_num_pulley_teeth	= 20;	//number of teeth on tming pulley or sproket
					//if pulley is not used default value is 1
const double x_mm_per_tooth	= 2.032;//effectively the pitch of the belt or chain in mm
					//if belt is not being used default value is 1
const double x_mm_per_thread	= 1;	//effectively the pitch of the thread in mm
					//if screw drive is not being used default value is 1
				
//--------------------------
//       Y - AXIS
//--------------------------
const double y_steps_per_rev	= 400;	//number of steps needed for motor to complete a full rotation
					//standard value is 200
const double y_microsteps	= 16;	//number of microsteps per step
					//if microstepping is not in use default value is 1
const double y_num_pulley_teeth	= 20;	//number of teeth on tming pulley or sproket
					//if pulley is not used default value is 1
const double y_mm_per_tooth	= 2.032;//effectively the pitch of the belt or chain in mm
					//if belt is not being used default value is 1
const double y_mm_per_thread	= 1;	//effectively the pitch of the thread in mm
					//if screw drive is not being used default value is 1
				
//--------------------------
//       Z - AXIS
//      (Print Bed)
//--------------------------
const double z_steps_per_rev	= 200;	//number of steps needed for motor to complete a full rotation
					//standard value is 200
const double z_microsteps	= 16;	//number of microsteps per step
					//if microstepping is not in use default value is 1
const double z_num_pulley_teeth	= 1;	//number of teeth on tming pulley or sproket
					//if pulley is not used default value is 1
const double z_mm_per_tooth	= 1;	//effectively the pitch of the belt or chain in mm
					//if belt is not being used default value is 1
const double z_mm_per_thread	= 2;	//effectively the pitch of the thread in mm
					//if screw drive is not being used default value is 1
				
//--------------------------
//       V - AXIS
//     (Storage Bin)
//--------------------------
const double v_steps_per_rev	= 200;	//number of steps needed for motor to complete a full rotation
					//standard value is 200
const double v_microsteps	= 16;	//number of microsteps per step
					//if microstepping is not in use default value is 1
const double v_num_pulley_teeth	= 1;	//number of teeth on tming pulley or sproket
					//if pulley is not used default value is 1
const double v_mm_per_tooth	= 1;	//effectively the pitch of the belt or chain in mm
					//if belt is not being used default value is 1
const double v_mm_per_thread	= 2;	//effectively the pitch of the thread in mm
					//if screw drive is not being used default value is 1
double stepsPerMM(char letter)
{
	int index;
	int i;
	
	//assign constants to arrys for easier manipulation
	double steps_per_rev[SIZE] = 	{x_steps_per_rev,	y_steps_per_rev,	z_steps_per_rev,	v_steps_per_rev};
	double microsteps[SIZE] = 	{x_microsteps,		y_microsteps,		z_microsteps,		v_microsteps};
	double num_pulley_teeth[SIZE] = {x_num_pulley_teeth,	y_num_pulley_teeth,	z_num_pulley_teeth,	v_num_pulley_teeth,};
	double mm_per_tooth[SIZE] = 	{x_mm_per_tooth,	y_mm_per_tooth,		z_mm_per_tooth,		v_mm_per_tooth};
	double mm_per_thread[SIZE] = 	{x_mm_per_thread,	y_mm_per_thread,	z_mm_per_thread,	v_mm_per_thread,};
	
	//check to make sure none of the values are 0 or negative
	//if they are change them to 1
	for(i=0; i<SIZE; i++)
	{
		if(steps_per_rev[i] <= 0)
			steps_per_rev[i] = 1;
			
		if(microsteps[i] <= 0)
			microsteps[i] = 1;
			
		if(num_pulley_teeth[i] <= 0)
			num_pulley_teeth[i] = 1;
			
		if(mm_per_tooth[i] <= 0)
			mm_per_tooth[i] = 1;
			
		if(mm_per_thread[i] <= 0)
			mm_per_thread[i] = 1;
	}
	
	//determine which axis is to be calculated
	//assign proper index value
	switch(letter)
	{
		case 'x':
		case 'X':
			index = 0;
			break;
		case 'y':
		case 'Y':
			index = 1;
			break;
		case 'z':
		case 'Z':
			index = 2;
			break;
		case 'v':
		case 'V':
			index = 3;
			break;
		default:
			index = -1;
	}
	
	//calculate and output value
	if(index>=0){
		return (steps_per_rev[index]*microsteps[index])/(num_pulley_teeth[index]*mm_per_tooth[index]*mm_per_thread[index]);
	}
	else	//case covers unrecognized axis letters
	{
		//printf("axis %c not recognized\n",letter);
		return 0;
	}
}

/* Commented out via macro */
#ifdef AXIS_CONFIG_MAIN
int main()
{
	//prints the value of each axis
	printf("x: %f\n", stepsPerMM('X'));
	printf("y: %f\n", stepsPerMM('Y'));
	printf("z: %f\n", stepsPerMM('z'));
	printf("v: %f\n", stepsPerMM('v'));
	printf("q: %f\n", stepsPerMM('q'));
	
	return 0;
}
#endif /* AXIS_CONFIG_MAIN */

#endif /* AXIS_CONFIG_C_ */
