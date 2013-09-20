#ifndef INIT_H
#define INIT_H

#include "intdef.h"

void initIdt ();
extern void initScreen ();
void setupIRQs ();
void prompt ();
void startPaging (uint32, uint32);
uint32 atapiInit ();
void initVFS ();
void initTimer ();
void initramfs ();
void initGDT ();

#endif
