/*
Embark Industries
Raspberry Pi I2C Interface for External Microcontroller
External Microcontroller: Arduino
Microcontroller Prog Name: ArduinoLaser
Controls the following
- IR Temp Sensor
- Laser Thermrister
- Relays for Sorensen

Rev3

Created By: Joseph Yosup Shim
Creation Date: 10/25/14

*/
#ifndef SLAVEMICROI2C_H_
int initializeSlaveMicroI2C(void);
int writeCommandSlaveMicroI2C(int dev, int command);
float readResultSlaveMicroI2C(void);
#endif