/*
 ============================================================================
 Name        : gpio_dummy.h
 Author      : Anthony Cargile
 Version     :
 Copyright   : Copyright 2014 Embark Industries
 Description : Fake GPIO routine definitions for building on non-Pis
 ============================================================================
 */
#ifndef GPIO_DUMMY
#define GPIO_DUMMY
 
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define PWM_OUTPUT 2

int wiringPiSetup();
void digitalWrite(int nope,int whocares);
void pinMode(int ignored, int neglected);

#endif /* GPIO_DUMMY */
