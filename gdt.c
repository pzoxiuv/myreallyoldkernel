#include "init.h"
#include "string.h"
#include "memory.h"
#include "task.h"

typedef struct gdtEntry {
	uint16 limitLow;
	uint16 baseLow;
	uint8 baseMiddle;
	uint8 access;
	uint8 limitHighAndFlags;
	uint8 baseHigh;
} __attribute__((packed)) gdtEntry_t;

typedef struct gdtPtr {
	uint16 limit;
	uint32 base;
} __attribute__((packed)) gdtPtr_t;

extern void flushGdt (uint32);
void gdtSetGate (uint32, uint32, uint32, uint8, uint8);

gdtEntry_t gdtEntries [4];

void initGDT () {
	kprintf ("Setting up GDT\n");
	
	gdtPtr_t ptr;

	ptr.limit = (sizeof (gdtEntry_t) * 4) - 1;
	ptr.base = (uint32) &gdtEntries;

	gdtSetGate (0, 0, 0, 0, 0);
	gdtSetGate (1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	
	gdtSetGate (2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	currentTss = kmalloc (sizeof (tss_t));
	currentTss->iopb = (uint16) sizeof (tss_t);
	currentTss->ss0 = 0x10;						//kernel datasegment descriptor is third entry in GDT
	asm volatile ("mov %0, esp" : "=r" (currentTss->esp0));
	gdtSetGate (3, (uint32) currentTss, sizeof (tss_t) - 1, 0x89, 0x40);

	flushGdt ((uint32) &ptr);
	asm volatile ("ltr ax" :: "a" (0x18));				//load TSS
}

void gdtSetGate (uint32 num, uint32 base, uint32 limit, uint8 access, uint8 gran) {
	gdtEntries [num].baseLow = (base & 0xFFFF);
	gdtEntries [num].baseMiddle = (base >> 16) & 0xFF;
	gdtEntries [num].baseHigh = (base >> 24) & 0xFF;

	gdtEntries [num].limitLow = limit & 0xFFFF;
	gdtEntries [num].limitHighAndFlags = (limit >> 16) & 0x0F;

	gdtEntries [num].limitHighAndFlags |= gran & 0xF0;	
	gdtEntries [num].access = access;
}
