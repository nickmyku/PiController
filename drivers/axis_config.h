#ifndef AXIS_CONFIG_H_
#define AXIS_CONFIG_H_
//header file for axis and motor configuration
//created by Nicholas Mykuylowycz
//created on May 1, 2014

//prototpye function declarations


double stepsPerMM(char letter);
/*
	function calculates the steps needed to move a given axis 1 mm

		returns the number of steps per mm of movement if the function is successful

		returns 0 if there was an error or an non existant axis was addressed
		
	letter designates the axis to caculate the number for, it accepts the following charcters
		
		'x' or 'X' - X axis
		
		'y' or 'Y' - Y axis
		
		'z' or 'Z' - Z axis (Print Bed)
		
		'v' or 'V' - V axis (Storage Bin)
		
	the parameters which are used to calculate the values are stored and constant doubles in the "axis_config.c" file
	
	the output value is caculated using the following equation
	
				steps_per_rev * microsteps
	steps_per_mm =	-----------------------------------------------
			num_pulley_teeth * mm_per_tooth * mm_per_thread
			
	if a belt drive is being used mm_per_thread is set to 1
	if a screw drive is being used num_pulley_teeth and mm_per_tooth are both set to 1
	
*/

#endif /* AXIS_CONFIG_H_ */
