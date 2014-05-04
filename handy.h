#ifndef HANDY_H_
#define HANDY_H_
/*
 * Handy.h
 * Author: Anthony Cargile <acargile@embarkindustries.com>
 * Function prototypes for the "handy" manual 3D Printer control app
 * Copyright 2014 Embark Industries, all rights reserved. 
 */

void move_pin(int pin, int speed);
void *lim_watchdog(void* ignored);

#endif /* HANDY_H_ */
