#ifndef VFS_H
#define VFS_H

#include "intdef.h"

void registerFS (uint8 *, uint32 (*)(uint8 *, uint32, uint8), uint32 (*)(uint32, uint32, uint8 *, int32), 
			uint32 (*)(uint32, uint32, uint8 *), uint32 (*)(uint8 *), uint32, uint8);
void registerRootFS (uint8 *, uint32 (*)(uint8 *, uint32, uint8), uint32 (*)(uint32, uint32, uint8 *, int32), 
			uint32 (*)(uint32, uint32, uint8 *), uint32 (*)(uint8 *), uint32, uint8);
uint32 open (uint8 *);
uint32 read (uint32, uint32, uint8 *);
uint32 lsDir (uint8 *);
uint32 seek (uint32, int32);

extern uint32 driveIRQ;

#endif

