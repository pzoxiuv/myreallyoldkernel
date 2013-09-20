#include "vfs.h"
#include "memory.h"
#include "string.h"
#include "elf.h"
#include "task.h"
#include "timer.h"

typedef struct {
	uint8 e_ident [16];
	uint16 e_type;
	uint16 e_machine;
	uint32 e_version;
	uint32 e_entry;
	uint32 e_phoff;
	uint32 e_shoff;
	uint32 e_flags;
	uint16 e_ehsize;
	uint16 e_phentsize;
	uint16 e_phnum;
	uint16 e_shentsize;
	uint16 e_shnum;
	uint16 e_shstrndx;
} elfHeader_t;

typedef struct {
	uint32 sh_name;
	uint32 sh_type;
	uint32 sh_flags;
	uint32 sh_addr;
	uint32 sh_offset;
	uint32 sh_size;
	uint32 sh_link;
	uint32 sh_info;
	uint32 sh_addralign;
	uint32 sh_entsize;
} section_t;

typedef struct {
	uint32 p_type;
	uint32 p_offset;
	uint32 p_vaddr;
	uint32 p_paddr;
	uint32 p_filesz;
	uint32 p_memsz;
	uint32 p_flags;
	uint32 p_align;
} programHeader_t;

typedef struct {
	uint32 r_offset;
	uint32 r_info;
} relocEntry_t;

typedef struct {
	uint32 st_name;
	uint32 st_value;
	uint32 st_size;
	uint8 st_info;
	uint8 st_other;
	uint16 st_shndx;
} symTable_t;

uint32 loadELF (uint8 *path) {
	uint32 i,j;//,k;
	uint32 fd = open (path);						//open file
	uint8 *buf = kmalloc (2048);						
	read (fd, 2048, buf);							//and read it

	uint8 *buf2 = kmalloc (sizeof (elfHeader_t));
	for (i=0; i<sizeof (elfHeader_t); i++) {				//read in the elf header
		buf2 [i] = buf [i];	
	}
	elfHeader_t *eHead = (elfHeader_t *)buf2;
	if (eHead->e_ident [0] != 0x7F || eHead->e_ident [1] != 'E' || eHead->e_ident [2] != 'L' || eHead->e_ident [3] != 'F') {
		return -1;							//doesn't contain the first magic bytes, not ELF
	}

	kprintf ("type %xs   machine %xs   version %xi   entry %xi\nphoff %xi   shoff %xi   flags %xi\nehsize %xs   phentsize %xs   phnum %xs   shentsize %xs\nshnum %xs   shstrndx %xs",
		eHead->e_type, eHead->e_machine, eHead->e_version, eHead->e_entry, eHead->e_phoff, \
		eHead->e_shoff, eHead->e_flags, eHead->e_ehsize, eHead->e_phentsize, eHead->e_phnum, \
		eHead->e_shentsize, eHead->e_shnum, eHead->e_shstrndx); 		

	uint8 *buf3 = kmalloc (eHead->e_shentsize);
	section_t **section = kmalloc (sizeof (section_t *) * eHead->e_shnum);	//allocate enough space for 'e_shnum' pointers to a section

	for (i=0; i<eHead->e_shnum; i++) {					//read in the sections
		for (j=0; j<eHead->e_shentsize; j++) {
			buf3 [j] = buf [eHead->e_shoff + j + (eHead->e_shentsize * i)];
		}
		section [i] = kmalloc (sizeof (section_t)); 
		kmemcpy ((void *)section [i], (void *)buf3, j);

		/*kprintf ("\nname %xi  type %xi  flags %xi  addr %xi\noffset %xi  size %xi  link %xi  info %xi\naddralign %xi  entsize %xi",
			section[i]->sh_name, section[i]->sh_type, section[i]->sh_flags, section[i]->sh_addr, section[i]->sh_offset, section[i]->sh_size, 
			section[i]->sh_link, section[i]->sh_info, section[i]->sh_addralign, section[i]->sh_entsize);*/
	}

	/*uint8 *strTable = kmalloc (section[eHead->e_shstrndx]->sh_size);
	for (i=section[eHead->e_shstrndx]->sh_offset; i<section[eHead->e_shstrndx]->sh_offset + section[eHead->e_shstrndx]->sh_size; i++) {
		strTable [i-section[eHead->e_shstrndx]->sh_offset] = buf [i];
	}

	symTable_t **symTable = (symTable_t **)-1;
	uint8 *buf6 = kmalloc (sizeof (symTable_t));
	for (i=0; i<eHead->e_shnum; i++) {
		if (section [i]->sh_type == 2) {				//section is SYMTAB section
			symTable = kmalloc (sizeof (symTable_t *) * section[i]->sh_size/sizeof (symTable_t));
			for (j=0; j<section[i]->sh_size/sizeof (symTable_t); j++) {
				for (k=0; k<sizeof (symTable_t); k++) {
					buf6 [k] = buf [section [i]->sh_offset+(j*sizeof(symTable_t))+k];
				}
				symTable [j] = kmalloc (sizeof (symTable_t));
				kmemcpy ((void *)symTable [j], (void *) buf6, k);
			}
			break;
		}
	}

	relocEntry_t *rel = kmalloc (sizeof (relocEntry_t));
	uint8 *buf5 = kmalloc (sizeof (relocEntry_t));	

	for (i=0; i<eHead->e_shnum; i++) {
		if (section [i]->sh_type == 9) {				//section is REL section
			for (j=0; j<sizeof (relocEntry_t); j++) {
				buf5 [j] = buf [section [i]->sh_offset+j];			
			}
			rel = (relocEntry_t *) buf5;
			break;
		}
	}

	uint32 addrToBeChanged = section [rel->r_info>>8]->sh_offset + rel->r_offset;
	uint32 newValue = buf [addrToBeChanged] + symTable [section [i]->sh_link]->st_value + 0x00400000;
	kprintf ("\n\naddr to be changed: %xi  new value: %xi\n\n", addrToBeChanged, newValue);
	buf [addrToBeChanged] = newValue;
	buf [addrToBeChanged+1] = newValue>>8;
	buf [addrToBeChanged+2] = newValue>>16;
	buf [addrToBeChanged+3] = newValue>>24;*/

	uint8 *buf4 = kmalloc (eHead->e_phentsize);
	programHeader_t **ph = kmalloc (sizeof (programHeader_t *) * eHead->e_phnum);
	uint32 highestHeader = 0;

	for (i=0; i<eHead->e_phnum; i++) {					//read in the sections
		for (j=0; j<eHead->e_phentsize; j++) {
			buf4 [j] = buf [eHead->e_phoff + j + (eHead->e_phentsize * i)];
		}
		ph [i] = kmalloc (sizeof (programHeader_t)); 
		kmemcpy ((void *)ph [i], (void *)buf4, j);

		kprintf ("\ntype %xi  offset %xi  vaddr %xi  paddr %xi\nfilesz %xi  memsz %xi  flags %xi  align %xi\n",
			ph[i]->p_type, ph[i]->p_offset, ph[i]->p_vaddr, ph[i]->p_paddr, ph[i]->p_filesz, ph[i]->p_memsz, ph[i]->p_flags, ph[i]->p_align);
		if (ph[i]->p_vaddr > ph[highestHeader]->p_vaddr) {
			highestHeader = i;
		}
	}
	
	for (i=0; i<((ph[highestHeader]->p_vaddr+ph[highestHeader]->p_memsz-0x00400000)/0x1000); i++) {
		mapTaskPage (getpid (), allocPage (), (void *) (0x00400000 + (0x1000*i)));
	}
	if (((ph[highestHeader]->p_vaddr + ph[highestHeader]->p_memsz)-0x00400000)%0x1000 != 0) {
		mapTaskPage (getpid (), allocPage (), (void *) (0x00400000 + (0x1000*i)));
	}
		//seek (fd, ph[1]->p_offset);
		read (fd, ph[0]->p_filesz, (uint8 *) ph[0]->p_vaddr);		
		//seek (fd, ph[1]->p_offset-ph[0]->p_filesz);
		read (fd, ph[0]->p_filesz, (uint8 *) ph[0]->p_vaddr);		
/*	for (i=0; i<eHead->e_phnum; i++) {
		seek (fd, ph[i]->p_offset);
		read (fd, ph[i]->p_filesz, (uint8 *) ph[i]->p_vaddr);		
		if (ph[i]->p_memsz > ph[i]->p_filesz) {
			kmemset ((void *) (ph[i]->p_vaddr+ph[i]->p_filesz), 0, ph[i]->p_memsz-ph[i]->p_filesz);		//zero any extra memory
		}
	}*/

	uint32 pageLine = ph [1]->p_vaddr & 0xFFFFF000;
	uint32 dataPadLen = pageLine - (ph [0]->p_vaddr + ph [0]->p_memsz);
	uint32 textPadLen = ph [1]->p_vaddr - pageLine;
	kmemcpy ((void *) (pageLine - dataPadLen), (void *) (pageLine + textPadLen), dataPadLen);			//fill in padding - see page 2-8 of ELF specs
	if (textPadLen > 0) {
		kmemcpy ((void *) (pageLine), (void *) pageLine - dataPadLen - textPadLen, textPadLen);				
	}
	//uint32 *end = (uint32 *)0x00400000;					//stores the end of the memory pool (for sbrk).  Overwrites parf of ELF header, but we're done with it anyway
	end = ph[1]->p_vaddr + ph[1]->p_memsz;
	//mapTaskPage (getpid (), allocPage (), (void *) 0x00800000);

	/*uint8 tmpStr [20];
	uint32 offset = 0, entry = 0;

	for (i=0; i<eHead->e_shnum; i++) {					//loop through str table, looking for the .text section
		j = section [i]->sh_name;
		while (strTable [j] != 0) {
			tmpStr [j-section[i]->sh_name] = strTable[j];
			j++;
		}
		tmpStr [j-section[i]->sh_name] = 0;
		kprintf ("section %d: %s\n", i, tmpStr);
		if (strcmp (tmpStr, (uint8 *)".data") == 0 || strcmp (tmpStr, (uint8 *)".data:") == 0 || 
				strcmp (tmpStr, (uint8 *)".text") == 0 || strcmp (tmpStr, (uint8 *)".text:") == 0) {
			kmemcpy ((void *) 0x00400000+offset, (void *) (buf+section [i]->sh_offset), section [i]->sh_size);
			offset += section [i]->sh_size;
			if (strcmp (tmpStr, (uint8 *)".text") == 0 || strcmp (tmpStr, (uint8 *)".text:") == 0) {
				entry = 0x00400000-section [i]->sh_size+offset;	
			}
		}
	}*/

	return eHead->e_entry;
}

uint32 loadBin (uint8 *path) {
	uint32 fd = open (path);
	uint8 *buf = kmalloc (2048);
	read (fd, 2048, buf);
	kmemcpy ((void *) 0x00400000, (void *) (buf), 2048);

	return 0;
}

uint32 exec (uint8 *path) {
	uint32 entry = loadELF (path);

	if (entry == -1) {
		mapTaskPage (getpid (), allocPage (), (void *) 0x00400000);
		loadBin (path);	
		entry = 0x00400000;
	}
	kprintf ("%xi\n", entry);
asm volatile ("xchg bx, bx");
	asm volatile ("jmp %0" :: "r" (entry));

	return 0;
}
