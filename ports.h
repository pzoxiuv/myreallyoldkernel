#ifndef PORTS_H
#define PORTS_H

#include "intdef.h"

extern void outb (uint16, uint8);
extern void outw (uint16, uint16);
extern void outd (uint16, uint32);
extern int8 inb (uint16);
extern int16 inw (uint16);
extern int32 ind (uint16);

#endif
