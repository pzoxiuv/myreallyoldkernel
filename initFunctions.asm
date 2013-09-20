%include "sysvars.asm"

extern clearScreen
extern updateCursor
extern kprintf

section .text

	global loadIdt
	global initScreen
	global findMemory
	global checkForMmap
	global enablePaging

enablePaging:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80010000
	mov cr0, eax
	mov esp, ebp
	pop ebp
	ret

loadIdt:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	lidt [eax]
	mov esp, ebp
	pop ebp
	ret

initScreen:
	call clearScreen
	
	xor eax, eax
	mov al, byte 0
	mov ah, byte 0
	mov [screen_text_x], ah
	mov [screen_text_y], al
	mov ah, 0x07
	mov [text_color], ah

	call updateCursor
	
	ret

findMemory:
	push ebp			;grab multiboot info struct from stack
	mov ebp, esp
	mov ebx, [ebp+8]

.bit0:
	mov eax, [ebx]			;check bit 1 in flags is set, meaning mem_lower and 
	and eax, 0b000000000001		;mem_upper are valid
	cmp eax, 1
	je .memAmount
	sub esp, 4
	mov eax, memLenNotValid
	mov [esp], eax
	call kprintf
	add esp, 4
	jmp .done			
	
.memAmount:
	add ebx, 4			;advance to low mem addr in multiboot struct
	mov eax, [ebx]			;move it into ecx 
	add ebx, 4			;advance to high mem addr in multiboot struct
	add eax, [ebx]			;add it to ecx
	mov ecx, eax
	shl ecx, 10			;mem amount in kb, multiply by 1024 for bytes
	shr eax, 10			;divide by 1024 to covert from kilobytes to megabytes
	push ecx
	sub esp, 12			;and print out mem info
	mov [esp], dword memMsg
	mov [esp+4], ecx
	mov [esp+8], eax
	call kprintf
	add esp, 12
	
	pop eax
.done:
	mov esp, ebp
	pop ebp
	ret

checkForMmap:
	push ebp			;grab multiboot info struct from stack
	mov ebp, esp
	mov ebx, [ebp+8]

	mov eax, [ebx]
	and eax, 0b000001000000		;check if bit 6 is set (mmap valid)
	cmp eax, 0b000001000000
	je .valid
	sub esp, 4
	mov eax, mmapNotValid
	mov [esp], eax
	call kprintf
	add esp, 4
	mov eax, 1
	
	mov esp, ebp
	pop ebp

	ret

.valid:
	mov eax, 0
	mov esp, ebp
	pop ebp
	ret

	memMsg		db	"Found %d bytes (%d MB) total system memory\n",0
	memMsg2		db	"\n%d bytes (%d MB) available to user\n",0
	memLenNotValid	db	"mem_upper and mem_lower not available\n",0
	mmapNotValid	db	"mmap not valid\n",0
