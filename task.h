#ifndef TASK_H
#define TASK_H

#include "intdef.h"

typedef struct tss {
	uint32 link;
	uint32 esp0, ss0;
	uint32 esp1, ss1;
	uint32 esp2, ss2;
	uint32 cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint32 es, cs, ss, ds, fs, gs;
	uint32 ldtr, iopb;
} __attribute__ ((packed)) tss_t;

extern uint32 readEip ();
void initTasks ();
uint32 fork ();
uint32 switchTasks (uint32, uint32, uint32, uint32, 
		uint32, uint32, uint32, uint32, uint32, uint32);
uint32 mapTaskPage (uint32, void *, void *);
uint32 getpid ();

tss_t *currentTss;

#endif
