#include "string.h"
#include "memory.h"
#include "init.h"

#define OPEN 0
#define READ 1

typedef struct fs {
	uint32 bus;
	uint8 device;
	uint8 *mountpoint;
	uint32 (*fsOpen) (uint8 *, uint32, uint8);
	uint32 (*fsRead) (uint32, uint32, uint8 *, int32);
	uint32 (*fsWrite) (uint32, uint32, uint8 *);
	uint32 (*listDir) (uint8 *);
	struct fs *nextFS;
} fs_t;

typedef struct file {
	uint32 fileNum;			//depending on FS, either LBA of file (ISO 9660) or pointer to dir entry (ramfs)
	int32 pos;
	fs_t *filesystem;
	struct file *nextFile;
} file_t;

fs_t *rootFS;
file_t *rootFile;

void initVFS () {
	kprintf ("\nInitializing VFS\n");
	initramfs ();
	rootFile = kmalloc (sizeof (file_t));
	rootFile->nextFile = (file_t *) -1;
}

void registerRootFS (uint8 *mountpoint, uint32 (*fsOpen) (uint8 *, uint32, uint8), uint32 (*fsRead) (uint32, uint32, uint8 *, int32), 
			uint32 (*fsWrite) (uint32, uint32, uint8 *), uint32 (*listDir) (uint8 *), uint32 bus, uint8 device) {
	rootFS = kmalloc (sizeof (fs_t));
	rootFS->mountpoint = mountpoint;
	rootFS->bus = bus;
	rootFS->device = device;
	rootFS->fsOpen = fsOpen;
	rootFS->fsRead = fsRead;
	rootFS->fsWrite = fsWrite;
	rootFS->listDir = listDir;
	rootFS->nextFS = (fs_t *) -1;
}

void registerFS (uint8 *mountpoint, uint32 (*fsOpen) (uint8 *, uint32, uint8), uint32 (*fsRead) (uint32, uint32, uint8 *, int32), 
			uint32 (*fsWrite) (uint32, uint32, uint8 *), uint32 (*listDir) (uint8 *), uint32 bus, uint8 device) {

	fs_t *newFS = kmalloc (sizeof (fs_t));
	newFS->mountpoint = mountpoint;
	newFS->bus = bus;
	newFS->device = device;
	newFS->fsOpen = fsOpen;
	newFS->fsRead = fsRead;
	newFS->fsWrite = fsWrite;
	newFS->nextFS = (fs_t *) -1;
	fs_t *currentFS = rootFS;
	while (currentFS->nextFS != (fs_t *) -1) {
		currentFS = currentFS->nextFS;
	}
	currentFS->nextFS = newFS;
}

uint32 open (uint8 *filename) {
	fs_t *currentFS = rootFS;
	file_t *newFile = kmalloc (sizeof (file_t));
	file_t *currentFile = rootFile;
	uint32 fileNumber = 1;

	while (currentFile->nextFile != (file_t *) -1) {
		fileNumber++;
		currentFile = currentFile->nextFile;
	}
	currentFile->nextFile = newFile;
	newFile->nextFile = (file_t *) -1;
	newFile->pos = 0;

	uint32 i;
	uint8 tempFilename [strlen (filename)];			//we will be modifying tempFilename, so we need it to be a seperate string from filename
	for (i=0; i<strlen (filename); i++) {
		tempFilename [i] = filename [i];
	}
	for (i=strlen (tempFilename); i>0; i--) {		//work backwards through file name, looking for match on mountpoint
		if (tempFilename [i] == '/') {			//we found a slash, check if everything after is a mountpoint
			tempFilename [i] = 0;			//so replace it with a null terminator and check the string to see if it matches any mountpoints
			currentFS = rootFS;
			do {
				currentFS = currentFS->nextFS;
				if (strcmp (currentFS->mountpoint, tempFilename) == 0) {	//we have a match, set this filesystem as the file's filesystem and return the file number
					newFile->filesystem = currentFS;
					newFile->fileNum = newFile->filesystem->fsOpen (filename, newFile->filesystem->bus, newFile->filesystem->device);
					return fileNumber;
				}	
			} while (currentFS->nextFS != (fs_t *)-1) ;	
		}
	}
	newFile->filesystem = rootFS;			//If the mtpoint wasn't found above, it must be just / - FIX! the root fs (/) should be seperate from the rootFS (first list item)
	newFile->fileNum = newFile->filesystem->fsOpen (filename, newFile->filesystem->bus, newFile->filesystem->device);
	return fileNumber;
}

uint32 read (uint32 fd, uint32 count, uint8 *buf) {
	if (rootFile->nextFile == (file_t *) -1) {
		return -1;
	}
	
	file_t *currentFile = rootFile->nextFile;
	uint32 i;
	for (i=1; i<fd; i++) {
		currentFile = currentFile->nextFile;
	}

	return currentFile->filesystem->fsRead (currentFile->fileNum, count, buf, currentFile->pos);
}

uint32 lsDir (uint8 *dir) {
	//rootFS->listDir (dir);
	file_t *currentFile = rootFile;
	uint32 i=0;
	while (currentFile->nextFile != (file_t *) -1) {
		currentFile = currentFile->nextFile;
		i++;
	 } 
	return 0;
}

uint32 seek (uint32 fd, int32 pos) {
	file_t *currentFile = rootFile;
	uint32 i;
	for (i=0; i<fd; i++) {
		currentFile = currentFile->nextFile;
	}
	currentFile->pos = pos;
	return 0;
}
