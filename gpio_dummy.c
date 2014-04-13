#ifndef TEST_BUILD
/*
 ============================================================================
 Name        : PIC.c
 Author      : Anthony Cargile
 Version     :
 Copyright   : Copyright 2014 Embark Industries
 Description : Silenceth the linker, lest it bitch.
 ============================================================================
 */
#include "gpio_dummy.h"

int wiringPiSetup() {
	return 0;
}

void digitalWrite(int nope,int whocares) {
	return;
}

void pinMode(int ignored, int neglected) {
	return;
}

int digitalRead(int godie) {
    return 0;
}

#endif /* test build */
