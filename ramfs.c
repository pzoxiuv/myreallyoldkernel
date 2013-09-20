#include "string.h"
#include "memory.h"
#include "vfs.h"

typedef struct dirEntry {
	struct dirEntry *nextDir;
	void *extent;
	uint8 *name;
	uint8 file;	//1 for yes,  0 for no
} dirEntry_t;

dirEntry_t *rootDir;

uint32 listDir (uint8 *);
uint32 mkDir (uint8 *, uint8 *);
uint32 mkFile (uint8 *, uint8 *);
uint32 writeFS (uint32, uint32, uint8 *);
uint32 readFS (uint32, uint32, uint8 *, int32);
uint32 openFS (uint8 *, uint32, uint8);
dirEntry_t *findDir (uint8 *);

void initramfs () {
	kprintf ("Initializing root directory (ramfs)\n");
	rootDir = kmalloc (sizeof (dirEntry_t));
	rootDir->extent = rootDir;
	rootDir->name = (uint8 *) ".";
	rootDir->file = 0;

	dirEntry_t *newDir = kmalloc (sizeof (dirEntry_t));
	rootDir->nextDir = newDir;
	newDir->nextDir = (dirEntry_t *) -1;
	newDir->extent = rootDir;
	newDir->name = (uint8 *) "..";
	newDir->file = 0;

	mkDir ((uint8 *) "/", (uint8 *) "test");
	mkDir ((uint8 *) "/test/", (uint8 *) "test2");
	mkDir ((uint8 *) "/test/", (uint8 *) "boink");
	mkFile ((uint8 *) "/test", (uint8 *) "boo.txt");

	uint8 *buf = (uint8 *) "Hello from boo.txt!";	
	writeFS (openFS ((uint8 *) "/test/boo.txt", 0, 0), 100, buf);

	registerRootFS ((uint8 *) "/", openFS, readFS, writeFS, listDir, 0, 0);
}

uint32 mkDir (uint8 *parentDirName, uint8 *dirName) {
	dirEntry_t *parentDir = findDir (parentDirName);
	
	dirEntry_t *newRootDir = kmalloc (sizeof (dirEntry_t));
	newRootDir->extent = newRootDir;
	newRootDir->name = (uint8 *) ".";
	newRootDir->file = 0;

	dirEntry_t *newDir = kmalloc (sizeof (dirEntry_t));
	newDir->extent = parentDir;
	newDir->name = (uint8 *) "..";
	newRootDir->nextDir = newDir;
	newDir->nextDir = (dirEntry_t *) -1;
	newDir->file = 0;
	
	newDir = kmalloc (sizeof (dirEntry_t));
	newDir->extent = newRootDir;
	newDir->name = dirName;
	newDir->nextDir = (dirEntry_t *) -1;
	newDir->file = 0;

	while (parentDir->nextDir != (dirEntry_t *) -1) {
		parentDir = parentDir->nextDir;	
	}
	
	parentDir->nextDir = newDir;

	return 0;
}

uint32 listDir (uint8 *dir) {
	dirEntry_t *currentDir = findDir (dir);
	do {
		kprintf ("%s ", currentDir->name);
		currentDir = currentDir->nextDir;
	} while (currentDir != (dirEntry_t *) -1);
	kprintf ("\n");
	return 0;
}

uint32 mkFile (uint8 *dirName, uint8 *name) {
	dirEntry_t *dir = findDir (dirName);
	
	dirEntry_t *newFile = kmalloc (sizeof (dirEntry_t));	//file is represented as a directory entry, just like a directory
	newFile->name = name;					//the only difference is it's extent points to the first block of the file, rather than another
	newFile->nextDir = (dirEntry_t *) -1;			//directory entry
	newFile->file = 1;
	newFile->extent = kmalloc (2048);
	
	while (dir->nextDir != (dirEntry_t *) -1) {
		dir = dir->nextDir;
	}	

	dir->nextDir = newFile;

	return 0;
}

uint32 writeFS (uint32 fileNum, uint32 count, uint8 *buf) {
	dirEntry_t *file = (dirEntry_t *) fileNum;
	uint8 *fileAddr = file->extent;

	int32 i;
	for (i=0; i<count; i++) {
		fileAddr [i] = buf [i];	
	}

	return 0;
}

uint32 readFS (uint32 fileNum, uint32 count, uint8 *buf, int32 pos) {
	dirEntry_t *file = (dirEntry_t *) fileNum;
	uint8 *fileAddr = file->extent;

	int32 i;

	for (i=0; i<count; i++) {
		buf [i] = fileAddr [i+pos];	
	}

	return 0;
}

uint32 openFS (uint8 *filename, uint32 bus, uint8 drive) {	//bus and drive are needed to maintain compatibility with ATAPI driver - figure out
	return 	(uint32) findDir (filename);			//a way to pass that info to ATAPI driver in a way which doesn't make this necessary for other drivers
}

dirEntry_t *findDir (uint8 *dir) {
	dirEntry_t *currentDir = rootDir;
	uint8 *dirName = kmalloc (25);
	dirName [0] = '/';
	dirName [1] = 0;

	if (strcmp (dir, dirName) == 0) {			//check special condition where dir requested is root directory
		return currentDir;
	}

	uint32 i, k = 0;
	for (i=0; i<strlen (dir); i++) {
		if (dir [i] == '/') {
			i++;
		}
		k = 0;
		while (dir [i] != 0 && dir [i] != '/') {	//continue looping until we get the whole dir name or to the end of the string
			dirName [k] = dir [i];
			k++;
			i++;
		}
		dirName [k] = 0;		
		while (strcmp (currentDir->name, dirName) != 0 && currentDir != (dirEntry_t *) -1) {
			currentDir = currentDir->nextDir;
		}
		if (strcmp (currentDir->name, dirName) != 0) {	//ran out of directories, but still didn't find desired directory
			return (dirEntry_t *) -1;			
		}
		if (currentDir->file == 1) {
			return currentDir;
		}
		currentDir = currentDir->extent;			
	}
	return currentDir;
}


