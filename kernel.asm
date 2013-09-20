%include "sysvars.asm"

extern _kmain
extern clearScreen
extern updateCursor
extern printf
global findMemory
global initScreen

section .text
	global kmain

kmain:
	push ebx			;preserve multiboot struct on stack
	call initScreen			
	call _kmain
	pop ebx				
	jmp hang2
	
	call findMemory	

hang2:
	hlt
	jmp hang2

	msg3	db	"%x", 0
	msg2	db	"Length of string: %d\n", 0
	msg	db	"Value of address returned: 0x%d\n", 0

initScreen:
	call clearScreen
	
	xor eax, eax
	mov al, byte 0
	mov ah, byte 0
	mov [screen_text_x], ah
	mov [screen_text_y], al

	call updateCursor
	
	ret

findMemory:

.bit0:
	mov eax, [ebx]			;check bit 1 in flags is set, meaning mem_lower and 
	and eax, 0b000000000001		;mem_upper are valid
	cmp eax, 1
	je .memAmount
	sub esp, 4
	mov eax, memLenNotValid
	mov [esp], eax
	call printf
	add esp, 4
	jmp .bit6			;This is a big problem - if we don't get 
					;to .memAmount, totalMemAmount is never set and the 						;memory manager gets screwed up
	
.memAmount:
	add ebx, 4			;advance to low mem addr in multiboot struct
	mov eax, [ebx]			;move it into ecx 
	add ebx, 4			;advance to high mem addr in multiboot struct
	add eax, [ebx]			;add it to ecx
	mov ecx, eax
	shl ecx, 10			;mem amount in kb, multiply by 1024 for bytes
	
	mov edx, totalMemAmount
	mov [edx], ecx			;store high mem addr in totalMemAmount

	shr eax, 10			;divide by 1024 to covert from kilobytes to megabytes
	push ebx
	sub esp, 12			;and print out mem info
	mov [esp], dword memMsg
	mov [esp+4], ecx
	mov [esp+8], eax
	call printf
	add esp, 12
	pop ebx

	jmp .createFirstEntry

.bit6:
	sub ebx, 8			;get back to flags
	mov eax, [ebx]
	and eax, 0b000001000000		;check if bit 6 is set (mmap valid)
	cmp eax, 0b000001000000
	je .mmap
	sub esp, 4
	mov eax, mmapNotValid
	mov [esp], eax
	call printf
	add esp, 4

.mmap:
	add ebx, 44			;get to mmap_length
	mov edi, [ebx]			;store mmap_length
	add ebx, 4			;get to mmap_addr
	mov eax, [ebx]			;store addr of mmap in eax
	add edi, eax			;move edi to end of mmap
.mmap_loop:
	add eax, 4 	
	mov ebx, mmapStr
	mov ecx, [eax]			;store base_addr in ecx
	mov edx, [eax+8]		;store length in edx
	mov esi, [eax+16]		;store type in esi
	push eax	
	add esp, 16			;print it all out
	mov [esp], ebx
	mov [esp+4], ecx
	mov [esp+8], edx
	mov [esp+12], esi
	call printf
	sub esp, 16
	pop eax
	add eax, [eax-4]		;add size of pair to advance to next pair
	cmp eax, edi
	jl .mmap_loop			;if we're not past the end of the mmap, continue

.createFirstEntry:			;creates first entry for memory allocation
	;mov eax, os_FirstMemAddr		
	;mov [eax], byte 0
	;add eax, 4
	;mov ebx, [totalMemAmount]
	;sub ebx, eax
	;mov [eax], ebx

	ret

	memMsg		db	"Found %d bytes (%d MB) total system memory\n",0
	memLenNotValid	db	"mem_upper and mem_lower not available\n",0
	mmapNotValid	db	"mmap not valid\n",0
	mmapStr		db	"base_addr: %x length: %d type: %d\n",0
