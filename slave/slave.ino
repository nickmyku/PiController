/*
 * File: slave.c
 * Description: Arduino I/O processor. It relays commands from I2C to I/O, quickly.
 * Author: Anthony Cargile
 * Copyright 2014 Embark Industries, all rights reserved. 
 */

#include <Wire.h>
#include "commands.h"
#define SLAVE_DEBUG 1

int cmd = 0; // global state storing last op code
int tx = 0; // buffer to send onto wire
boolean halt = false;

void setup() {
#ifdef SLAVE_DEBUG
  Serial.begin(9600);
#endif
  Wire.begin(SLAVE_BUS_ID);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop(){
  delay(100);
}

void receiveData(int byteCount){
    while(Wire.available()) {
      cmd = Wire.read();
#ifdef SLAVE_DEBUG
      Serial.print("data received: ");
      Serial.print(cmd);
#endif
      if(halt) {
        if(cmd == OP_UNHALT) {
          halt = false;
        }
        return; // even if freshly unhalted - perf op
      }
      switch(cmd) {
        case OP_HALT:
          halt = true;
          return;
          break;
	case OP_HELLO:
	  tx = OP_HELLO; // handshake
	  halt = false;
	  return;
	  break;
        default:
#ifdef SLAVE_DEBUG
          Serial.print("No handler for command: ");
          Serial.print(cmd);
#endif
          break;  
      }
    }
}

void sendData() {
  if(halt) return;
  
  if(tx != 0) Wire.write(tx);
}
