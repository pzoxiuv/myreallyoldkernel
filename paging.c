#include "string.h"
#include "memory.h"
#include "init.h"
#include "multiboot.h"

typedef struct multiboot_memory_map {
	uint32 size;
	uint32 base_addr_low,base_addr_high;
	uint32 length_low,length_high;
	uint32 type;
} multiboot_memory_map_t;

extern int32 findMemory (multiboot_info_t*);
extern void enablePaging (uint32 *);
extern int32 checkForMmap (multiboot_info_t*);
void displayMmap (void *);
void generatePageTable ();
uint32 bitSet (uint32, uint8);
uint32 bitClear (uint32, uint8);
uint32 bitTest (uint32, uint8);

uint8 *endOfPage, *nextAddress;
uint32 pageBitmap [1024];	//Fix!!! Should not have to reserve all 1024 ints unless we actually need all of them.  But if we just allocate a pointer, real hardware messes up...
uint32 numTables, numHeapPages;

/*
 * Sets bit in target, returns target
 */

uint32 bitSet (uint32 target, uint8 bit) {
	return target | (1 << bit);		//shifts a 1 into specified bit, or's it with target to set bit
}

/*
 * Clears bit in target, returns target
 */

uint32 bitClear (uint32 target, uint8 bit) {
	return target & ~(1 << bit);		//shits 1 into specified bit, inverse is and'd with target to clear bit
}

/*
 * Tests bit in target, returns 1 if set, 0 if not set
 */

uint32 bitTest (uint32 target, uint8 bit) {
	return (target >> bit) & 1;		//shifts specified bit into bit 0 position in target, and's target with 1 to clear everything else and returns target
}

/*
 * Allocates specified number of consecutive pages
 */

void *allocPage () {
	uint32 bit = 0;
	while (1) {
		if (bit % 32 == 0 || bit == 0) {
			if (pageBitmap [bit/32] == 0xFFFFFFFF) {	//this uint is all taken, move to next one			
				bit += 32;
				continue;
			}
		}
		if (bitTest (pageBitmap [bit/32], bit) == 0) {
			pageBitmap [bit/32] = bitSet (pageBitmap [bit/32], bit % 32);
			kmemset ((void *) (bit * 0x1000), 0, 0x1000);
			return (uint32 *) (bit * 0x1000);
			
		}
		else {
			bit++;
		}		
	}

	return 0;
}

/*
 * Frees `pages` number of pages, starting at `start` page.
 */

void freePage (void *start, uint32 pages) {
	uint32 i;
	uint32 first = (uint32) start;
	for (i=0; i<pages; i++) {	
		pageBitmap [(first+i)/4096/32] = bitClear (pageBitmap [(first+i)/4096/32], (first+i) % (4096*32));
	}
}

void initHeap () {
	uint8 *newPage = allocPage ();
	//mapPage (pageDirectory, (void *)newPage, (void *)0xD0000000, 0, 1);
	nextAddress = newPage;
	endOfPage = nextAddress + 0x1000;
	numHeapPages = 1;
}

void *kmalloc (uint32 bytes) {
	if (nextAddress + bytes > endOfPage) {
		uint8 *newPage = allocPage ();
		//mapPage (pageDirectory, (void *)newPage, (void *) ((0x1000*numHeapPages) + 0xD0000000), 0, 1);
		numHeapPages++;
		nextAddress = newPage;
		endOfPage = nextAddress + 0x1000;
	}

	nextAddress += bytes;
	return nextAddress - bytes;
}

void mapPage (uint32 *pageDir, void *physicalAddr, void *virtualAddr, uint8 user, uint8 writable) {
	asm volatile ("invlpg [%0]" :: "r" (virtualAddr));
	
	uint32 pgDirIndex = (uint32) virtualAddr >> 22;
	uint32 pgTableIndex = (uint32) virtualAddr >> 12 & 0x03FF;			//10 1's in binary
	
	uint32 *pageTable = (uint32 *) (pageDir [pgDirIndex] & 0xFFFFFF00);		//this should be zero when cloning pg dir, it appears to not be
	//kprintf ("%xi %d %d\n", pageTable, pgDirIndex, pgTableIndex);
	if (pageTable == 0) {
		pageTable = allocPage ();	
	}
	pageTable [pgTableIndex] = (uint32) physicalAddr | 1 | 2*writable | 4*user;	//sets to present, if writable is not 0, it will or it with 0b10 for writable, same for user
	pageDir [pgDirIndex] = (uint32) pageTable;
	pageDir [pgDirIndex] |= 3;

	pgDirIndex = (uint32) physicalAddr >> 22;
	pgTableIndex = (uint32) physicalAddr >> 12 & 0x03FF;
	pageTable = (uint32 *) (pageDir [pgDirIndex] & 0xFFFFFF00);

	if ((uint32) physicalAddr < 0x1F4000 && (pageTable [pgTableIndex] & 0xFFFFFF00) == (uint32) physicalAddr) {	//this page was identity mapped, unmap it
		asm volatile ("invlpg [%0]" :: "r" (physicalAddr));	
		pageTable [pgTableIndex] = (uint32) 0;	
	}

	//kprintf ("%xi %xi %xi\n", pageDir, physicalAddr, virtualAddr);
}

uint32 *clonePageDirectory (uint32 *sourcePgDir) {
	uint32 *newPageDirectory = allocPage ();
	//uint32 *newPageTable = allocPage ();
	kmemset ((void *)newPageDirectory, 0, 0x1000);
	//mapPage (pageDirectory, (void *)newPageDirectory, (void *)newPageDirectory, 0, 1);
	//mapPage (pageDirectory, (void *)newPageTable, (void *)newPageTable, 0, 1);

	uint32 i;
	for (i=0; i<1024; i++) {
		if (sourcePgDir [i] == 0 || i == 896) {				//we want to link everything except the stack and input buf
			continue;
		}
		if (pageDirectory [i] == sourcePgDir [i]) {	 				//if it was in the original kernel pg dir, just copy it
			newPageDirectory [i] = sourcePgDir [i];
		}
	}

	uint32 *newStack = allocPage ();
	uint32 *srcStack = (uint32 *) (sourcePgDir [896] & 0xFFFFF000);		///////////fix ebps, add more stack space
	srcStack = (uint32 *) (srcStack [0] & 0xFFFFF000);

	kmemcpy ((void *)newStack, (void *)srcStack, 0x1000);
	mapPage (newPageDirectory, (void *)newStack, (void *)0xE0000000, 0, 1);

	return newPageDirectory;
}


/*
 * Displays memory map obtained from GRUB and sets read/write bit for each page entry in page directory according
 * to whether the memory section is used or available (according to the memory map).
 */

void displayMmap (void* mbt_ptr) {
	multiboot_info_t* mbt = (multiboot_info_t*) mbt_ptr;
	uint32 i;	//,baseAddr		//for looping through and setting page table entries
	uint32 bit = 0;			//for setting bit map while setting page table entries

	kprintf ("\nMemory map:\n\n");
 
	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbt->mmap_addr);
	while ((int32) mmap < (int32) (mbt->mmap_addr + mbt->mmap_length)) {
		
		setTextColor (0x09);
		kprintf ("0x%xi        ", mmap->base_addr_low);
		setTextColor (0x0f);
		kprintf ("%d bytes", mmap->length_low);

		if (mmap->type == 1) {
			//baseAddr = (mmap->base_addr_low & 0xFFFFF000) / 0x1000;
			//*(pageDirectory + (0x1000 + baseAddr)) |= 3;					//Set at least one page table entry, even if length/4096 < 1
			bit++;
			for (i=0; i<mmap->length_low/0x1000+1; i++) {					//Otherwise, start at 4kb aligned address/1024
				//*(pageDirectory + (0x1000 + i + baseAddr)) |= 3;			//Set unused page to present, kernel-mode, r/w
				if (mmap->base_addr_low + i*0x1000 > 0x0000FFFF && 			//even though BIOS thinks this area is free, it actually contains the kernel and page directory
					mmap->base_addr_low + i*0x1000 < (uint32) pageDirectory + 0x1000*numTables) {	//so we want to reserve it in the bitmap
					pageBitmap [bit/32] = bitSet (pageBitmap [bit/32], bit%32);
				}									//pageDirectory + 0x1000 = pageDirectory[0], pgDir+0x2000=pgDir[1], etc.
				bit++;									//pgDir + 0x1001 = pgDir[0][1], pgDir+0x2004 = pgDir[1][3], etc.
			} 										
			setTextColor (0x0a);								
			kprintf ("        usable\n");
		}
		else {
			//baseAddr = (mmap->base_addr_low & 0xFFFFF000) / 0x1000;
			//*(pageDirectory + (0x1000 + baseAddr)) &= 1;
			pageBitmap [bit/32] = bitSet (pageBitmap [bit/32], bit%32);			//set bit in (bit/32)'th uint of bitmap to indicate it's not free
			bit++;
			for (i=0; i<mmap->length_low/0x1000; i++) {					
				pageBitmap [bit/32] = bitSet (pageBitmap [bit/32], bit%32);					//set bit in (bit/32)'th uint of bitmap to indicate it's not free
				bit++;
				//*(pageDirectory + (0x1000 + i + baseAddr)) &= 1;			//same as before, but now set as present, kernel-mode read only					
			}
			setTextColor (0x0c);
			kprintf ("        unusable\n");
		}
		mmap = (multiboot_memory_map_t*) ((uint32)mmap + mmap->size
			+ sizeof (uint32));

	}
	kprintf ("\n");
}

void generatePageTable () {

	if (numTables % 32 != 0) {
		numTables += numTables % 32;						//we want to make sure we get at least one table, and don't leave any memory off at the end
	}
	
	kmemset ((uint8 *)pageBitmap, 0, numTables/32);				//zero bitmap - length is pages/32, or numTables*1024/32, or numTables/32
	uint32 i, j;		
	for (i=5; i<16; i++) {
		pageBitmap [i] = 0xFFFFFFFF;						//Assume everything from video memory -> 2 MB is used by kernel
	}

	pageBitmap [0] = bitSet (pageBitmap [0], 0);					//reserve first 4096 bytes for input buffer, sysvars, etc.

	pageDirectory = allocPage ();							//pageDirectory addr = kernel start + size of kernel (25K in this case)
	uint32 *pageTable = allocPage ();

	kprintf ("Generating page directory with %d tables\n", numTables);

	for (i=0; i<1; i++) {								//id map first 2 mb
		for (j=0; j<512; j++) {
			pageTable [j] = (j*0x1000) | 3;						
		}
		pageDirectory [i] = (uint32) pageTable;				//mark all memory as kernel-mode, r/w, and present.  If we want, we can redefine it as user-mode
		pageDirectory [i] |= 3;							//when we need to (when we actually start a user-program)
	}

	initHeap ();
}

void moveStack (uint32 oldStackStart) {

	uint32 newStackStart = (uint32) allocPage () + 0x1000;		//allocate 2 pages (8192 bytes) for stack
	kmemset ((void *)(newStackStart-0x1000), 0, 0x1000); 				//allocate the next page (fix - we're pretty sure they'll be consecutive but we should check anyway)
	uint32 oldEsp, oldEbp;
	asm volatile ("mov %0, esp; mov %1, ebp" : "=r" (oldEsp), "=r" (oldEbp));	

	uint32 offset = newStackStart - oldStackStart;		
	uint32 newEsp = oldEsp + offset;
	uint32 newEbp = oldEbp + offset;

	kmemcpy ((void *)newEsp, (void *)oldEsp, oldStackStart-oldEsp);
	
	uint32 i, tmp, *tmp2;
	for (i = newStackStart; i > newEsp; i -= 4) {
		tmp = * (uint32 *)i;
		if (tmp > oldEsp && tmp < oldStackStart) {				//if this i value was within the bounds of the old stack, it could be an ebp pointing to somewhere in the stack 	
			tmp += offset;
			tmp2 = (uint32 *)i;
			*tmp2 = tmp;
		}
	}

	mapPage (pageDirectory, (void *)(newStackStart - 0x1000), (void *) 0xE0000000, 0, 1);

	asm volatile ("mov cr3, %0" :: "r" (pageDirectory));

	offset = 0xE0001000 - newStackStart;						//new offset, from top of stack (at 0xE0001000) to new phys stack start
	oldStackStart = newStackStart;
	newStackStart = 0xE0001000;
	oldEsp = newEsp;
	newEsp = newEsp + offset;
	newEbp = newEbp + offset;

	for (i = newStackStart-4; i > 0xE0000000; i -= 4) {
		tmp = * (uint32 *)i;
		if (tmp > oldEsp && tmp < oldStackStart) {				//if this i value was within the bounds of the old stack, it could be an ebp pointing to somewhere in the stack 	
			tmp += offset;
			tmp2 = (uint32 *)i;
			*tmp2 = tmp;
		}
	}

	asm volatile ("mov esp, %0; mov ebp, %1;" :: "r" (newEsp), "r" (newEbp));
}

void startPaging (uint32 mbtPtr, uint32 oldStackStart) {

	multiboot_info_t* mbt = (multiboot_info_t*) mbtPtr;

	uint32 bytesRAM = findMemory (mbt);				//find total mem amount (from mbt), create first entry in memory manager			
	numTables = ((bytesRAM >> 10) / 4096) + 1;
	
	if (checkForMmap (mbt) == 0) {
		displayMmap ((void *)mbt);
	}

	setTextColor (0x0F);
	generatePageTable ();						//shr by 10 to convert bytes->kilobytes and divide by 4096, then add 1 to determine number of tables we need	

	kprintf ("Enabling paging\n");
	enablePaging (pageDirectory);
	
	void *inputBuf = allocPage ();
	kmemset (inputBuf, 0, 0x1000);
	mapPage (pageDirectory, inputBuf, (void *)0xD0000000, 0, 1);

	moveStack (oldStackStart);
}
