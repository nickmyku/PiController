#ifndef LIMIT_SW_H_
#define LIMIT_SW_H_
//header file for limit switch functions
//created by Nicholas Mykuylowycz
//created on May 26, 2014

//prototpye function declarations

void set_dir(int axis, int dir);
/*
	function checks limit switch state before setting direction
	
	axis accepts integer to set (defined in commands.h)
		X_DIR_PIN
		Y_DIR_PIN
		Z_DIR_PIN
		V_DIR_PIN
		
	dir accepts integer to set dirction (defined in commands.h)
		DIR_NEG
		DIR_POS

*/


#endif /* LIMIT_SW_H_ */