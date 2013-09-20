#!/bin/bash

export PATH=$PATH:/usr/local/cross/bin/
nasm -f elf32 -o sysvars.o sysvars.asm
nasm -f elf32 -o memory.o memory.asm
nasm -f elf32 -o string.o string.asm
nasm -f elf32 -o screen.o screen.asm
nasm -f elf32 -o initFunctions.o initFunctions.asm
nasm -f elf32 -o loader.o loader.asm
nasm -f elf32 -o interrupts.o interrupts.asm
nasm -f elf32 -o misc.o misc.asm
i586-pc-myos-gcc -m32 -o ckernel.o -c ckernel.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -mno-red-zone -g
i586-pc-myos-gcc -m32 -o keyboard.o -c keyboard.c -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -mno-red-zone -g
i586-pc-myos-gcc -m32 -o interruptsc.o -c interrupts.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -mno-red-zone -g
i586-pc-myos-gcc -m32 -o input.o -c input.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -mno-red-zone -g
i586-pc-myos-gcc -m32 -o prompt.o -c prompt.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -mno-red-zone -g
i586-pc-myos-gcc -m32 -o paging.o -c paging.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -mno-red-zone -g
i586-pc-myos-gcc -m32 -o atapi.o -c atapi.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -mno-red-zone -g
i586-pc-myos-gcc -m32 -o vfs.o -c vfs.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -mno-red-zone -g
i586-pc-myos-gcc -m32 -o ramfs.o -c ramfs.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -g
i586-pc-myos-gcc -m32 -o timer.o -c timer.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -g
i586-pc-myos-gcc -m32 -o gdt.o -c gdt.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -g
i586-pc-myos-gcc -m32 -o task.o -c task.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -g
i586-pc-myos-gcc -m32 -o elf.o -c elf.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel -g
i586-pc-myos-gcc -m32 -o syscalls.o -c syscalls.c -Wall -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -masm=intel
i586-pc-myos-ld -T link.ld -o kernel.bin loader.o initFunctions.o screen.o string.o memory.o sysvars.o interrupts.o misc.o ckernel.o interruptsc.o input.o keyboard.o prompt.o paging.o atapi.o vfs.o ramfs.o timer.o task.o gdt.o elf.o syscalls.o
cat stage1 stage2 pad kernel.bin > floppy.img
qemu -cpu qemu32 -m 512 -monitor stdio  -cdrom bootable.iso -kernel kernel.bin
#qemu -m 64 -monitor stdio -fda floppy.img

