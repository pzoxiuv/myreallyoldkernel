;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;MISC.ASM;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section .text:
	global outb
	global outw
	global outd
	global inb
	global inw
	global ind
	global readEip
	global flushGdt

;flushes gdt, replacing the one setup by GRUB with our own
flushGdt:
	mov eax, [esp+4]
	lgdt [eax]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:.flush
.flush:
	ret

;pops eip into eax (return value) and jmps to eip
readEip:
	pop eax
	jmp eax

;outputs a byte of data to specified port
;in: char port, char data data
;out: nothing

outb:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	out dx, al
	mov esp, ebp
	pop ebp
	ret	

;outputs a word of data to specified port
;in: char port, short data
;out: nothing

outw:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	out dx, ax
	mov esp, ebp
	pop ebp
	ret

;outputs a dword of data to specified port
;in: char port, int data
;out: nothing

outd:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	out dx, eax
	mov esp, ebp
	pop ebp
	ret

;gets a byte of data to specified port
;in: char port, char data data
;out: nothing

inb:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	in al, dx
	mov esp, ebp
	pop ebp
	ret	

;gets a word of data to specified port
;in: char port, short data
;out: nothing

inw:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	in ax, dx
	mov esp, ebp
	pop ebp
	ret

;gets a dword of data to specified port
;in: char port, int data
;out: nothing

ind:
	xor edx, edx
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edx, [ebp+8]
	mov eax, [ebp+12]
	in eax, dx
	mov esp, ebp
	pop ebp
	ret
