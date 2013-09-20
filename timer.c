#include "string.h"
#include "ports.h"
#include "init.h"
#include "task.h"
#include "timer.h"

uint32 tick = 0;
uint32 seconds = 0;

void initTimer () {
	kprintf ("Initializing PIT at 100 Hz\n");	//100 Hz = 100 fires/second = 1 fire/millisecond

	outb (0x43, 0x36);
	outb (0x40, (1193280 / 100) & 0xFF);		//PIT frequency, divided by desired frequency, then get lower byte
	outb (0x40, ((1193280 / 100) >> 8) & 0xFF);	//same, for upper byte
}

void timerTick (uint32 eflags, uint32 esp, uint32 eip, uint32 ebp, uint32 esi, uint32 edi,
		uint32 edx, uint32 ecx, uint32 ebx, uint32 eax) {
	tick++;
	//kprintf ("tick");
	if (tick % 20 == 0) {
		//kprintf ("SWITCH PLACES!\n");
		//switchTasks (eflags, esp, eip, ebp, esi, edi, edx, ecx, ebx, eax);
	}
}

void sleep (uint32 milliseconds) {
	uint32 initialTicks = tick;
	while (tick-initialTicks < milliseconds) {}
}
