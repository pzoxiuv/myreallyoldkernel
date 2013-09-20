#include "string.h"
#include "ports.h"
#include "vfs.h"
#include "init.h"

typedef struct idt_entry {
	uint16 base_low;
	uint16 sel;
	uint8 always0;
	uint8 flags;
	uint16 base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_pointer {
	uint16 size;
	uint32 base;
} __attribute__((packed))idt_pointer_t;

extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10 ();
extern void isr11 ();
extern void isr12 ();
extern void isr13 ();
extern void isr14 ();
extern void isr15 ();
extern void isr16 ();
extern void isr17 ();
extern void isr18 ();
extern void irq15 ();
extern void irq1 ();
extern void irq0 ();
extern void irq ();
extern void sys ();
extern void loadIdt ();

void idtSetGate (uint8, uint32, uint16, uint8);
void remapPIC ();
void interrupt (uint32, uint32, uint32, uint32);

idt_entry_t idt [256];
idt_pointer_t idtp;
uint32 driveIRQ = 0;

void initIdt () {
	idtp.size = (sizeof (idt_entry_t) * 256) - 1;
	idtp.base = (uint32) &idt;

	kprintf ("Filling IDT...");

	idtSetGate (0, (uint32) isr0, 0x08, 0x8E);
	idtSetGate (1, (uint32) isr1, 0x08, 0x8E);
	idtSetGate (2, (uint32) isr2, 0x08, 0x8E);
	idtSetGate (3, (uint32) isr3, 0x08, 0x8E);
	idtSetGate (4, (uint32) isr4, 0x08, 0x8E);
	idtSetGate (5, (uint32) isr5, 0x08, 0x8E);
	idtSetGate (6, (uint32) isr6, 0x08, 0x8E);
	idtSetGate (7, (uint32) isr7, 0x08, 0x8E);
	idtSetGate (8, (uint32) isr8, 0x08, 0x8E);
	idtSetGate (9, (uint32) isr9, 0x08, 0x8E);
	idtSetGate (10, (uint32) isr10, 0x08, 0x8E);
	idtSetGate (11, (uint32) isr11, 0x08, 0x8E);
	idtSetGate (12, (uint32) isr12, 0x08, 0x8E);
	idtSetGate (13, (uint32) isr13, 0x08, 0x8E);
	idtSetGate (14, (uint32) isr14, 0x08, 0x8E);
	idtSetGate (15, (uint32) isr15, 0x08, 0x8E);
	idtSetGate (16, (uint32) isr16, 0x08, 0x8E);
	idtSetGate (17, (uint32) isr17, 0x08, 0x8E);
	idtSetGate (18, (uint32) isr18, 0x08, 0x8E);

	uint32 i;
	for (i = 19; i < 256; i++) {
		idtSetGate (i, (uint32) isr0, 0x08, 0x8E);
	}
	idtSetGate (0xF0, (uint32) sys, 0x08, 0x8E);
	kprintf ("loading IDT\n");

	loadIdt (&idtp);
}

void idtSetGate (uint8 num, uint32 base, uint16 sel, uint8 flags) {

	idt [num].sel = sel;
	idt [num].flags = flags;
	idt [num].always0 = 0;
	idt [num].base_low = (base & 0xFFFF);
	idt [num].base_high = (base >> 16) & 0xFFFF;
}

void setupIRQs () {
	remapPIC ();
	uint32 i;
	for (i = 32; i < 48; i++) {
		idtSetGate (i, (uint32) irq, 0x08, 0x8E);
	}
	idtSetGate (32, (uint32) irq0, 0x08, 0x8E);	
	idtSetGate (33, (uint32) irq1, 0x08, 0x8E);
	idtSetGate (46, (uint32) irq15, 0x08, 0x8E);
	idtSetGate (47, (uint32) irq15, 0x08, 0x8E);
	kprintf ("Enabling interrupts\n");
	asm volatile ("sti");
}

void remapPIC () {
	outb (0x20, 0x11);
	outb (0xA0, 0x11);
	outb (0x21, 0x20);
	outb (0xA1, 0x28);
	outb (0x21, 0x04);
	outb (0xA1, 0x02);
	outb (0x21, 0x01);
	outb (0xA1, 0x01);
	outb (0x21, 0x00);
	outb (0xA1, 0x00);
}

void interrupt (uint32 interruptNum, uint32 errorCode, uint32 eip, uint32 cr2) {
	int8 *isr [] = { "Division by zero", "Debug", "Non maskable interrupt", "Breakpoint", "Into Dected Overflow", "Out of bounds", "Invalid opcode",
				  "No coprocessor", "Double fault", "Coprocessor segment overrun", "Bad TSS", "Segment not present", "Stack fault", "General protection fault", "Page fault",
				  "Unknown interrrupt", "Coprocessor fault", "Alignment check", "Machine check", };
	setTextColor (0x04);
	kprintf ("\n%s exception", isr [interruptNum]);
	kprintf ("\nerror code: %xi", errorCode);
	kprintf ("\neip: %xi", eip); 
	kprintf ("\ncr2: %xi", cr2); 
	for (;;);
}

void drive () {
	//kprintf ("drive IRQ");
	driveIRQ = 1;
}
