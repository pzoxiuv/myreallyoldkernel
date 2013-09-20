#include "string.h"
#include "ports.h"

extern void backspace ();

uint8 scLower [] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '0',
		    	      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
		              '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0, 0, 0, 0,
			       'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0 , ' ',};

uint8 scUpper [] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '0',
		    	      '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
		              '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 0, 0, 0, 0,
			       'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0 , ' ',};

int32 shiftDown = 0;

void keyboard () {
	uint8 *input = (uint8 *) 0xD0000000;
	uint8 inch = inb (0x60);
	
	if (inch == 0x2a || inch == 0x36) { 				//shift down
		shiftDown = 1;
		return;
	}
	else if (inch == (0x2a+0x80) || inch == (0x36 + 0x80)) { 	//shift up
		shiftDown = 0;
		return;
	}
	
	if (inch == 0x0E && input[0] != 0) {
		input[0]--;						//backspace
		backspace ();
		return;
	}
	
	if (inch < 0x58) {						//we only want keydown events, not keyup
		if (shiftDown == 0) {
			if (input [1] == 1) {			//if something has indicated they want input, save keys to buffer
				input[input [0]+2] = scLower [inch];
			}
			kprintf ("%c", scLower [inch]);
		}
		else {
			if (input [1] == 1) {
				input[input [0]+2] = scUpper [inch];
			}
			kprintf ("%c", scUpper [inch]);
		}
		input[0]++;
		if (inch == 0x1C) {					//enter, we're done getting input
			input [1] = 2;				//let everyone know we're done getting input
			input [0] = 0;				//reset counter
		}
	}
}
