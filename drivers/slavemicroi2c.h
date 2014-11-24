/*
Embark Industries
Raspberry Pi I2C Interface for External Microcontroller
External Microcontroller: Arduino
Microcontroller Prog Name: ArduinoLaser
Controls the following
- IR Temp Sensor (this will be communicated with by the pi directly)
- Laser Thermrister
- Relays for Sorensen

Rev3

Created By: Joseph Yosup Shim
Creation Date: 10/25/14

*/
#ifndef SLAVEMICROI2C_H_

int initializeArduino(void);
int enableLaser(int dev, bool enabled);
float getLaserTemp(int dev);
//int writeCommandSlaveMicroI2C(int dev, int command);


#endif