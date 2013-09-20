#include "string.h"
#include "memory.h"
#include "init.h"
#include "vfs.h"

void add ();
void subtract ();
void divide ();
void multiply ();
void memdump ();
void ls ();

uint8 *currentDir;

void prompt () {
	currentDir = kmalloc (300);
	currentDir = (uint8 *) "/test";
	uint8 command [100];
	while (1) {
		kprintf (">");	
		getstr (command);
		if (strcmp (command, (uint8 *) "add") == 0)
			add ();
		else if (strcmp (command, (uint8 *) "sub") == 0)
			subtract ();
		else if (strcmp (command, (uint8 *) "div") == 0)
			divide ();
		else if (strcmp (command, (uint8 *) "mul") == 0)
			multiply ();
		else if (strcmp (command, (uint8 *) "memdump") == 0)
			memdump ();
		else if ((strcmp (command, (uint8 *) "ls") == 0) || (strcmp (command, (uint8 *) "dir") == 0)) {
			ls ();
		}
		else
			kprintf ("Unknown command\n");
		kmemset (command, 0, 100);				//zero "command"
	}
}

void ls () {
	lsDir (currentDir);
}

void add () {
	kprintf ("First Number: ");
	int32 num1 = getint ();
	kprintf ("Second Number: ");
	int32 num2 = getint ();
	kprintf ("%d+%d=%d\n", num1, num2, num1+num2);
}

void subtract () {
	kprintf ("First Number: ");
	int32 num1 = getint ();
	kprintf ("Second Number: ");
	int32 num2 = getint ();
	kprintf ("%d-%d=%d\n", num1, num2, num1-num2);
}

void divide () {
	kprintf ("First Number: ");
	int32 num1 = getint ();
	kprintf ("Second Number: ");
	int32 num2 = getint ();
	kprintf ("%d/%d=%d\n", num1, num2, num1/num2);
}

void multiply () {
	kprintf ("First Number: ");
	int32 num1 = getint ();
	kprintf ("Second Number: ");
	int32 num2 = getint ();
	kprintf ("%d*%d=%d\n", num1, num2, num1*num2);
}

void memdump () {
	uint8 buf [10];
	kprintf ("Address: ");
	getstr (buf);
	uint8 *address = (uint8 *)stox (buf);
	kprintf ("Number of bytes: ");
	uint32 bytes = getint ();
	uint32 i = 0;
	setTextColor (0x09);
	kprintf ("0x%xi ", address);
	setTextColor (0x0F);
	while (i < bytes) {
		kprintf (" %xb", *address);
		i++;
		address++;
		if (i % 16 == 0  && i < bytes) {
			setTextColor (0x09);
			kprintf ("\n0x%xi ", address);
			setTextColor (0x0F);
		}	
	}
	kprintf ("\n");	
	setTextColor (0x07);
}
