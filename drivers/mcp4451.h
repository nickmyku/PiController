#ifndef MCP4451_H_
//header file of driver for the mcp4451 digital potentiometer chip
//used to control sorensen DCS60-18 supplies
//Nicholas Mykulowycz
//Created May 26, 2014

//prototype functions
int intitializeMCP();
int setVoltage(int device, int supply, double voltage);
int setCurrent(int device, int supply, double current);
double getVoltage(int device, int supply);
double getCurrent(int device, int supply);
int scaleTo255(double value, double max_value);
double scaleToHuman(int value, double max_value);
void setPotValue(int device, int pot_addr, int value);
int readPotValue(int device, int pot_addr);

#endif  /* MCP4451_H_ */