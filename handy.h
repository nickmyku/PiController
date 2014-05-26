#ifndef HANDY_H_
#define HANDY_H_
/*
 * Handy.h
 * Author: Anthony Cargile <acargile@embarkindustries.com>
 * Function prototypes for the "handy" manual 3D Printer control app
 * Copyright 2014 Embark Industries, all rights reserved. 
 */


// use this to provide dev-only feedback. See printDebug() too.
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)


// don't call this - call DEBUG() instead
void printDebug(const char *fmt, const int line, const char *file, ...);

void move_pin(int pin, int speed, double steps_per_mm);
void *lim_watchdog(void* ignored);
//void set_dir(int axis, int dir); //moved to drivers/limit_sw

#endif /* HANDY_H_ */
