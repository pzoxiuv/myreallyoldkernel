#ifndef STRING_H
#define STRING_H

#include "intdef.h"

extern void kprintf (const char *fmt, ...);
extern int32 stoi (uint8 *);
extern int32 strcmp (uint8 *, uint8 *);
extern int32 strlen (uint8 *);
extern int8 *substr (uint8 *, uint32, uint32);
extern int32 itox (uint32);
extern int32 stox (uint8 *);
extern void setTextColor (uint8);
uint8 getch ();
void getstr (uint8 *);
int32 getint ();

#endif
