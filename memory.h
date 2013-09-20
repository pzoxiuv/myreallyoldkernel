#ifndef MEMORY_H
#define MEMORY_H

#include "intdef.h"

extern void kmemset (void *, uint8, uint32);
extern void kmemcpy (void *, void *, uint32);
void *allocPage ();
void freePage (void *, uint32);
void *kmalloc (uint32);
uint32 *clonePageDirectory ();
void mapPage (uint32 *, void *, void *, uint8, uint8);

uint32 *pageDirectory;

#endif
