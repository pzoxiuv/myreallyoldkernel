#include "string.h"

uint8 *inputBuf = (uint8 *) 0xD0000000;

uint8 getch () {
	inputBuf [1] = 1;							//let keyboard driver know we want to save input

	while (inputBuf [1] != 2) {}					//until keyboard driver sets new value, wait for enter

	return inputBuf [2];	
}

void getstr (uint8 *buf) {
	inputBuf [1] = 1;

	while (inputBuf [1] != 2) {}

	uint32 i = 2;
	while (inputBuf [i] != '\n') {
		buf [i-2] = inputBuf [i];
		i++;
	}

	inputBuf [i] = 0;							//add null terminator	
}

int32 getint () {
	uint8 numStr [10];						//ints can only be 10 digits long
	getstr (numStr);							
	
	return stoi (numStr);
}
