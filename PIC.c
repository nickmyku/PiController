/*
 ============================================================================
 Name        : PIC.c
 Author      : Anthony Cargile
 Version     :
 Copyright   : Copyright 2014 Embark Industries
 Description : Entry point for PIC manufacturing utility.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#ifndef TEST_BUILD
// building on non-Pi, include dummy GPIO interface:
#include "gpio_dummy.h"
#else
// real deal, include real GPIO library files:
#include <wiringPi.h>
#endif

// globals:
char *file; // stores the whole Gcode file

int main(int argc, char **argv) {
	// initialize pins:
	if(wiringPiSetup() == -1) {
		printf("Pin setup failed!\n(Do you even root, bro?)\n");
		exit(1);
	}

	if(argc <= 1) {
		printf("Your argument is invalid.\nAnd you're a towel.\n");
		exit(1);
	}

	FILE *fp = NULL;
	if((fp = fopen(argv[1], "r")) == NULL) {
		printf("That file doesn't even exist.\nNice try, asshole.\n");
		exit(1);
	}

	pinMode(1, /*PWM_*/OUTPUT); // pin 1 (header pin #12) for laser
	pinMode(0, OUTPUT); // x-axis
	pinMode(2, OUTPUT); // y-axis
	pinMode(3, OUTPUT); // z-axis

	struct stat st;
	stat(argv[1], &st);
	long size = st.st_size;

	if((file = malloc(size*sizeof(char))) == NULL) {
		printf("I don't have enough memory for this...uh...\n...?\n");
		exit(1);
	}

	if(fread(file, sizeof(char), size, fp) != size) {  // read file into memory
		printf("File I/O error\nI can't read too much too good no-how.");
		exit(1);
	}
	fclose(fp);

	char *cmd;
	cmd = strtok(file,"\n"); // split file into commands, one per line
	int line = 0; // for debugging line numbers
	char *parts; // reserved for G1 command parsing
	while(cmd != NULL) {
		line++;
		if(cmd[0] == 'M') {
			if(!strcmp(cmd, "M03")) {
				// laser on
				digitalWrite(1, HIGH);
			}
			else if(!strcmp(cmd, "M05")) {
				// laser off
				digitalWrite(1, LOW);
			}
			else {
				printf("%d: The fuck is this shit? (%s)\n", line, cmd);
			}
		}
		else if(cmd[0] == 'G') {
			if(cmd[1] == '1') { // good old G1
				char parts[10][10]; // limit 10 parts, 10 chars each
				int partc = 0, parti = 0; // indices
				int len = strlen(cmd); // perf op
				int i;
				for(i = 2; i < len; i++) {
					if(cmd[i] == ' ') {
						if(parti == 0) continue;
						partc++;
						parti = 0;
					}
					else {
						parts[partc][parti] = cmd[i];
					}
				}

				// parts[][] now holds args of G1 command
				for(; partc >= 0; partc--) { // reverse iterate
					switch(parts[partc][0]) { // axis
						case 'X':
							break;

						case 'Y':
							break;
						default:
							printf("%d: %c is an axis to you?\nFunny, now put the crackpipe down.\n", line, parts[partc][0]);
							break;
					}
				}
			}
			else {
				printf("%d: Go home, '%s'. You're drunk.\n", line, cmd);
			}
		}
		cmd = strtok((char*)NULL, "\n");
	} // while(cmd != NULL)

	return EXIT_SUCCESS;
}
