/*
 * Name          : y.PIC.h
 * Created on    : Feb 9, 2014
 * Author        : Anthony Cargile
 * Description   : Prototypes for the main PIC program
 * Copyright     : 2014 Embark Industries, all rights reserved. 
 */

#ifndef Y_PIC_H_
#define Y_PIC_H_

#define laseron()  laser(100.0)
#define laseroff() laser(0.0)
#define ON_A_SHARK_HEAD 0

int sanity(int argc, char **argv);
void calibrate();
int printOut();
void printDebug(const char *fmt, const int line, const char *file, ...);
void eprintf(const char *fmt, ...);
void axis_updated();
int laser(double power);
int cleanup();

#endif /* Y_PIC_H_ */

