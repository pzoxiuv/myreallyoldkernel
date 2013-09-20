#include "string.h"
#include "init.h"
#include "multiboot.h"
#include "vfs.h"
#include "task.h"
#include "timer.h"
#include "memory.h"
#include "elf.h"

void _kmain (multiboot_info_t* mbt, uint32 initialEsp) {

	initScreen ();							//clear screen, update cursor
	
	setTextColor (0x0f);
	kprintf ("Well, we've turned on at least\n"); 			//print welcome message
	
	startPaging ((uint32) mbt, initialEsp);
	initGDT ();
	initIdt ();
	setupIRQs ();
	initVFS ();
	atapiInit ();
	initTimer ();
	initTasks ();

	uint8 *buf = kmalloc (2048);
	uint32 fd = open ((uint8 *) "/BOOT/GRUB/HELLO.C");
	asm ("cli;hlt");	
	read (fd, 100, buf);
	kprintf ("\nContents of /test/boo.txt:\n%s\n", buf);
	read (fd, 100, buf);
	kprintf ("\nContents of /test/boo.txt:\n%s\n", buf);

	setTextColor (0x07);

	//uint32 pid = fork ();
	//if (pid == 0) {
	//	exec ((uint8 *) "/BOOT/GRUB/HELLO.O");
	//}

	exec ((uint8 *) "/BOOT/GRUB/HELLO.O");
	prompt ();

	for (;;);
}
