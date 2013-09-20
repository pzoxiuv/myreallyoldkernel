#include "vfs.h"
#include "string.h"
#include "memory.h"
#include "task.h"
#include "elf.h"

uint32 syscl (uint32 callNum, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5) {

	kprintf ("\n[syscall] %xi %xi %xi %xi %xi\n", callNum, callNum, arg1, arg2, arg3);
	switch (callNum) {
		case 1:						//write (int file, char *ptr, int len);		
			kprintf ("%s", (uint8 *) arg2);			
			return arg3;				//return len
		case 2:						//wait (int *status); //not yet implemented!
			return -1;
		case 3:						//unlink (char *name); //not yet implemented!
			return -1;			
		case 4:						//times (struct tms *buf); //not yet implemented!	
			return -1;				
		case 5:						//stat (const char *file, struct stat *st); //not yet implemented!
			return 0;				
		case 6:						//sbrk (int inc);
			mapTaskPage (getpid (), allocPage, (void *) arg1);
			return 0;
		case 7:						//read (int file, char *ptr, int len);
			read (arg1, arg3, (uint8 *)arg2);
			return 0;
		case 8:						//open (const char *name, int flags, int mode);
			return open ((uint8 *) arg1);
		case 9:						//lseek (int file, int ptr, int dir); //not yet implemented!
			return 0;
		case 10:					//link (char *old, char *new); //not yet implemented!
			return -1;
		case 11:					//kill (int pid, int sig); //not yet implemented!
			return -1;
		case 12:					//isatty (int file); //not yet implemented!
			return 1;
		case 13:					//getpid ()
			return getpid ();			
		case 14:					//fstat (int file, struct stat *st); //not yet implemented!
			return 0;				
		case 15:					//fork
			return -1;
		case 16:					//execve (char *name, char **argv, char **env); //not yet implemented!
			return -1;
		case 17:					//close (int file); //not yet implemented!
			return -1;				
		case 18:					//_exit
			return -1;
		case 19:
			return end;
		default:
			break;
	}
	return 0;
}
