#ifndef HANDY_H_
#define HANDY_H_
/*
 * Handy.h
 * Author: Anthony Cargile <acargile@embarkindustries.com>
 * Function prototypes for the "handy" manual 3D Printer control app
 * Copyright 2014 Embark Industries, all rights reserved. 
 */


void move_pin(int pin, int speed, double steps_per_mm);
void *lim_watchdog(void* ignored);
//void set_dir(int axis, int dir); //moved to drivers/limit_sw

#endif /* HANDY_H_ */
