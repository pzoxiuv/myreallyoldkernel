#include "string.h"
#include "ports.h"
#include "vfs.h"
#include "memory.h"
#include "timer.h"

uint8 drives [3];		//0=nonexistant, 1=PATA, 2=SATA, 3=ATAPI

uint32 atapiRead (uint32, uint32, uint8 *, int32);
uint32 atapiSeek (uint32, uint32, uint8);
void readCD ();
uint32 findFile (uint8 *, uint32, uint8);
uint32 atapiListDir (uint8 *);

void readCD () {
	uint8 buf [2048];
	atapiRead (0x10, 2048, buf, 0);
	int32 i;//, k=0;
	kprintf ("\nVolume descriptor type: ");
	kprintf ("%xb", buf[0]);
	kprintf ("\nSystem identifier:  ");
	for (i=1; i<6; i++) {
		kprintf ("%c", buf [i]);
	}
	kprintf ("\nName of system that can write to sectors 0x00-0x0F: ");
	for (i=8; i<40; i++) {
		kprintf ("%c", buf[i]);
	}
	kprintf ("\nVolume identification:  ");
	for (i=40; i<72; i++) {
		kprintf ("%c", buf [i]);
	}
	kprintf ("\nLBA of root directory entry: %d", (uint32) ((buf [156+2]) | (buf [157+3] << 8) | (buf [158+4] << 16) | (buf [159+5] << 24)));
	
	atapiRead (25, 2048, buf, 0);
	uint32 addr = 0;
	uint32 dirEntrySize;
	
	while (1) {
		dirEntrySize = (uint32) buf [addr];
		if (dirEntrySize == 0)
			break;
		kprintf ("\nLength of entry: %d", dirEntrySize);	//directory entries are one after another - add start+length to get to next one
		kprintf ("\nLBA: %d", (uint32) ((buf [addr+2]) | (buf [addr+3] << 8) | (buf [addr+4] << 16) | (buf [addr+5] << 24)));	//LBA of file in entry
		kprintf ("\nSize of file identifier: %d\nFile/directory name: ", (uint32) buf [addr+32]);
		uint32 size = (uint32) buf [addr+32];
		for (i=0; i<size; i++) {
			kprintf ("%c", buf [addr+i+33]);
		}
		addr += dirEntrySize;
	}
	/*k=0;
	atapiRead (174, 2048, buf);
	for (i=0; i<2048; i++) {
		kprintf ("%c", buf[i]);
		if (k>40) {
			k=0;
			kprintf ("\n");		
		}
	}*/
}

uint32 findFile (uint8 *filename, uint32 bus, uint8 device) {
	uint32 dirLBA, addr = 0, dirEntrySize, file;
	uint8 *buf = kmalloc (2048);	//file: 1=yes, 0=no (directory)
	uint8 *dirName = 0;

	atapiRead (0x10, 2048, buf, 0);		//read primary volume descriptor
	kprintf ("\nread one done\n");
	dirLBA = (uint32) ((buf [156+2]) | (buf [157+3] << 8) | (buf [158+4] << 16) | (buf [159+5] << 24));	//find LBA of root directory entries
	//kprintf ("dirlba %d\n", dirLBA);
	atapiRead (dirLBA, 2048, buf, 0);	//and load it into buf
	kprintf ("\nread two done\n");

	uint32 i, k;
	for (i=0; i<strlen (filename); i++) {		//consider passing in strlen (filename), it could save quite a few calls to strlen
		if (filename [i] == '/') {
			k=0;
			i++;
			while (filename [i] != '/' && i < strlen (filename)) {
				dirName [k] = filename [i];
				if (dirName [k] == '.') {
					file = 1;
				}
				i++;
				k++;		
			}
			if (file == 1) {
				dirName [k] = ';';
				dirName [k+1] = '1';
				k+=2;				
			}
			dirName [k] = 0;
			//kprintf ("\n%s %d", dirName, file);
			addr = 0;
			dirEntrySize = buf [addr];
			while (dirEntrySize != 0) {					//loop through directory entries until we find our target
				if (buf [addr+32] == strlen (dirName)) {		//if it's the right length, we check if the chars match
					//kprintf (" right length ");
					for (k=0; k<strlen (dirName); k++) {
						if (dirName [k] != buf [addr+33+k]) {	//chars don't match, break out of for loop and check the next entry
							addr += dirEntrySize;
							dirEntrySize = buf [addr];
							k = -1;
							break;					
						}
					}
					//kprintf (" %d ", k);
					if (k != -1) {					//we found a match, this entry is our directory
						dirLBA = (uint32) ((buf [addr+2]) | (buf [addr+3] << 8) | (buf [addr+4] << 16) | (buf [addr+5] << 24));	//load next LBA
						//kprintf (" %d ", dirLBA);
						if (file == 1) {			//LBA is for file, return the LBA
							return dirLBA;
						}
						atapiRead (dirLBA, 2048, buf, 0);		//otherwise, it was for a directory, so load the next entry
						break;					//break out of while loop
					}
				}
				else {
					addr += dirEntrySize;				//name length doesn't match, so move on to next entry
					dirEntrySize = buf [addr];
				}
			}
			//kprintf ("%s\n", filename);
			i--;
		}
	}

	return -1;									//if we get here we didn't find the file, so return -1
}

uint32 atapiSeek (uint32 lba, uint32 currentBus, uint8 currentDrive) {
	driveIRQ = 0;
	//uint8 in;
	uint32 maxByteCount = 2048;
	uint8 lbaArray [4] = {(lba & 0xFF), ((lba >> 8) & 0xFF), ((lba >> 16) & 0xFF), ((lba >> 24) & 0xFF) };
	outb (currentBus+6, currentDrive);				//select drive
	inb (currentBus+7); inb (currentBus+7); inb (currentBus+7); inb (currentBus+7);	//400ns delay after select
	outb (currentBus+1, 0x0);					//PIO mode
	outb (currentBus+4, maxByteCount & 0xFF);
	outb (currentBus+5, maxByteCount >> 8);
	outb (currentBus+7, 0xA0);
	while ((inb (currentBus+7) & 0x80) != 0);			//wait until BSY clears
	while ((inb (currentBus+7) & 0x08) != 8);			//wait until DRQ is set
	outw (currentBus, (0 << 8) | 0x2B);				//byte 0 - seek extended command and byte 1 (flags) ***higher byte is first, lower second***
	outw (currentBus, ((lbaArray [2] << 8) | lbaArray [3]));	//byte 2 (MSB of LBA) and byte 3
	outw (currentBus, ((lbaArray [0] << 8) | lbaArray [1]));	//byte 4 and byte 5 (LSB of LBA)
	outw (currentBus, (0 << 8) | 0);				//byte 6 (reserved) and byte 7 (reserved
	outw (currentBus, (0 << 8) | 0);				//byte 8 (reserved) and byte 9 (reserved bits 7-2, bit1=flag, bit0=link)
	outw (currentBus, (0 << 8) | 0);
	while ((inb (currentBus+7) & 0x80) != 0);			//wait until BSY clears
	while (driveIRQ == 0) {}

	return 0;
}

uint32 atapiRead (uint32 lba, uint32 count, uint8 *buf, int32 pos) {
	int32 i;
	uint32 currentBus;
	uint8 currentDrive;
	for (i=0; i<4; i++) {
		if (drives [i] == 3) {
			switch (i) {						//device is ATAPI, figure out what it's bus/drive numbers are
				case 0:
					currentBus = 0x1F0;
					currentDrive = 0xA0;
					break;
				case 1:
					currentBus = 0x1F0;
					currentDrive = 0xB0;
					break;
				case 2:
					currentBus = 0x170;
					currentDrive = 0xA0;
					break;
				case 3:
					currentBus = 0x170;
					currentDrive = 0xB0;
					break;
				default:;
			}
		}
	}
	driveIRQ = 0;
	//uint8 status;
	uint16 maxByteCount = 2048;
	lba += pos/2048;
	uint8 lbaArray [4] = {(lba & 0xFF), ((lba >> 8) & 0xFF), ((lba >> 16) & 0xFF), ((lba >> 24) & 0xFF) };

	//kprintf ("\nReading from ATAPI device\n");
	outb (currentBus+6, currentDrive);
	inb (currentBus+7); inb (currentBus+7); inb (currentBus+7); inb (currentBus+7);	//400ns delay after select
	outb (currentBus+1, 0x0);					//PIO mode
	outb (currentBus+4, maxByteCount & 0xFF);
	outb (currentBus+5, maxByteCount >> 8);
	outb (currentBus+7, 0xA0);
	while ((inb (currentBus+7) & 0x80) != 0);			//wait until BSY clears
	while ((inb (currentBus+7) & 0x08) != 8);			//wait until DRQ is set
	outw (currentBus, (0 << 8) | 0xA8);				//byte 0 - read command (byte 0 of READ EXTENDED) (read (12)) and byte 1 (flags) ***higher byte is first, lower second***
	outw (currentBus, ((lbaArray [2] << 8) | lbaArray [3]));	//byte 2 (MSB of LBA) and byte 3
	outw (currentBus, ((lbaArray [0] << 8) | lbaArray [1]));	//byte 4 and byte 5 (LSB of LBA)
	outw (currentBus, (0 << 8) | 0);				//byte 6 (MSB of transfer length) and byte 7
	outw (currentBus, (1 << 8) | 0);				//byte 8 and byte 9 (LSB of transfer length)
	outw (currentBus, (0 << 8) | 0);				//byte 10 (reserved) and byte 11 (control)
	while ((inb (currentBus+7) & 0x80) != 0);			//wait until BSY clears	
	while ((inb (currentBus+7) & 0x08) != 8);			//wait until DRQ is set
	while (driveIRQ == 0) {}
	uint32 size = ((uint32) inb (currentBus+4)) | (((uint32) inb (currentBus+5) << 8));
	uint16 in;
	//kprintf ("\nlba %d  ", lba);
	//for (i=0; i<pos%2048; i+=2) {
	//	inw (currentBus);
	//}
	i=0;
	//kprintf ("%xi %xi\n", buf, count);
	while (i<count) {
		if (i > size) {
			return size;		
		}
		in = inw (currentBus);
		buf [i] = in & 0xFF;
		if (i+1 == count) {					//if i+1 is the count, then count is odd and we just wrote our last byte - return now
			while (i < size) {
				inb (currentBus);
				i++;
			}
			return count;
		}
		buf [i+1] = in >> 8;
		if (i<0x10) {
			kprintf ("%xs", in);			
		}
		i+=2;
		if ((i+(pos%2048))%2048 == 0 && i<count) {
			pos = -i;
			lba++;
			//kprintf ("\nlba %d  ", lba);
			lbaArray [0] = lba & 0xFF;
			lbaArray [1] = (lba >> 8) & 0xFF;
			lbaArray [2] = (lba >> 16) & 0xFF;
			lbaArray [3] = (lba >> 24) & 0xFF;
			outb (currentBus+1, 0x0);					//PIO mode
			outb (currentBus+4, maxByteCount & 0xFF);
			outb (currentBus+5, maxByteCount >> 8);
			outb (currentBus+7, 0xA0);
			while ((inb (currentBus+7) & 0x80) != 0);			
			while ((inb (currentBus+7) & 0x08) != 8);			
			outw (currentBus, (0 << 8) | 0xA8);				
			outw (currentBus, ((lbaArray [2] << 8) | lbaArray [3]));	
			outw (currentBus, ((lbaArray [0] << 8) | lbaArray [1]));	
			outw (currentBus, (0 << 8) | 0);				
			outw (currentBus, (1 << 8) | 0);				
			outw (currentBus, (0 << 8) | 0);				
			while ((inb (currentBus+7) & 0x80) != 0);			
			while ((inb (currentBus+7) & 0x08) != 8);			
			size += ((uint32) inb (currentBus+4))  | (((uint32) inb (currentBus+5) << 8));	
			//kprintf ("size: %d\n", size);				
		}
	}
	
	while (i < size) {
		inw (currentBus);
		i += 2;
	}

	return size;
}

uint32 atapiWrite () {
	return 1;
}

uint32 atapiListDir (uint8 *dir) {
	return 1;
}

uint32 atapiInit () {
	uint32 i, j, k=0, currentBus;
	uint8 currentDrive, result;
	currentBus = 0x1F0;
	currentDrive = 0xA0;
	for (i=0; i<2; i++) {
		for (j=0; j<2; j++) {			
			outb (currentBus+6, currentDrive);		//select drive
			//outb (currentBus+6, 0x08);			//software reset
			//while ((inb (currentBus+7) & 0x80) != 0);	//wait until BSY clears
			//outb (currentBus+6, currentDrive);		//select drive
			inb (currentBus+7); inb (currentBus+7); inb (currentBus+7); inb (currentBus+7);	//400ns delay after select
			outb (currentBus+2, 0);				//zero sectorcount, LBAlo, LBAmid, LBAhi 
			outb (currentBus+3, 0);
			outb (currentBus+4, 0);
			outb (currentBus+5, 0);
			outb (currentBus+7, 0xEC);			//send IDENTIFY command
			while ((inb (currentBus+7) & 0x80) != 0);	//wait until BSY clears
			//kprintf ("Trying IDENTIFY command\n");
			result = inb (currentBus+7);
			if (result == 0) {
				drives [k] = 0;
				//kprintf ("No device (result is zero)\n");
				k++;
				currentDrive = 0xB0;
				continue;
			}
			if ((inb (currentBus+1) & 0x04) == 4) {		//ABRT (abort) set in error register, assume device is ATAPI
				drives [k] = 3;
				kprintf ("Found ATAPI device...");
				k++;
				currentDrive = 0xB0;
				continue;
			}
			uint8 cl = inb (currentBus+4);		//if it was set, device is either ATAPI or SATA, so check values of cl and ch to determine
			uint8 ch = inb (currentBus+5);

			kprintf ("Device is ", result);
			//kprintf ("Also, here is the error register: 0x%xb\n", inb (currentBus+1));
			if (cl == 0x14 && ch == 0xEB) {
				drives [k] = 3;
				kprintf ("PATAPI\n");
			}
			else if (cl == 0x69 && ch == 0x96) {
				drives [k] = 3;
				kprintf ("SATAPI\n");
			}
			else if (cl == 0 && ch == 0) {
				drives [k] = 1;
				kprintf ("PATA\n");
			}
			else if (cl == 0x3C && ch == 0xC3) {
				drives [k] = 2;
				kprintf ("SATA\n");
			}
			else {
				kprintf ("Huh? %xb %xb\n", cl, ch);
			}
			k++;
			currentDrive = 0xB0;
		}
		currentBus = 0x170;
		currentDrive = 0xA0;
	}

	for (i=0; i<4; i++) {						//now get some info about the ATAPI device
		if (drives [i] != 3) {				
			continue;
		}
		switch (i) {						//device is ATAPI, figure out what it's bus/drive numbers are
			case 0:
				currentBus = 0x1F0;
				currentDrive = 0xA0;
				break;
			case 1:
				currentBus = 0x1F0;
				currentDrive = 0xB0;
				break;
			case 2:
				currentBus = 0x170;
				currentDrive = 0xA0;
				break;
			case 3:
				currentBus = 0x170;
				currentDrive = 0xB0;
				break;
			default:;
		}
		outb (currentBus+6, currentDrive);		//select ATAPI device
		inb (currentBus+7); inb (currentBus+7); inb (currentBus+7); inb (currentBus+7);	//400ns delay after select
		outb (currentBus+2, 0);				//zero sectorcount, LBAlo, LBAmid, LBAhi 
		outb (currentBus+3, 0);
		outb (currentBus+4, 0);
		outb (currentBus+5, 0);
		if (drives [i] == 3) {
			outb (currentBus+7, 0xA1);		//if drive is ATAPI, send IDENTIFY PACKET DEVICE command
		}

		while ((inb (currentBus+7) & 0x80) != 0);	//wait until BSY bit is clear
		while ((inb (currentBus+7) & 0x08) != 8);	//then wait until DRQ sets

		uint16 buf [256];
		uint16 model [100];

		if ((inb (currentBus+7) & 0x01) != 1) {		//if ERR is clear, we're ready to read!
			j=0;
			while ((inb (currentBus+7) & 0x08) == 8) {
				buf [j] = inw (currentBus);			
				j++;
			}
			for (k=0; k<j; k++) {
				if (k > 26 && k < 47) {
					model [k-27] = (buf [k] << 8) | (buf [k] >> 8);		//reverse bytes
				}
			}
			model [20] = 0;
			kprintf ("device name: %s\n", model);
		}

		registerFS ((uint8 *) "/BOOT", findFile, atapiRead, atapiWrite, atapiListDir, currentBus, currentDrive);
		//atapiRead (196, 100, buf, 0);
	}

	return 0;
}
