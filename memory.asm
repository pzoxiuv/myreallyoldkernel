;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;MEMORY.ASM;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "sysvars.asm"

section .text:
	global kmemset
	global kmemcpy

;in: pointer to destination, value memory is to be set to, how many bytes are to be set
;out: nothing

kmemset:
	xor ebx, ebx
	xor ecx, ecx
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov ebx, [ebp+12]
	mov ecx, [ebp+16]

.setData:
	mov [eax], bl
	inc eax
	loop .setData
	
	mov esp, ebp
	pop ebp
	ret

;in: pointer to destination, pointer to source, count

kmemcpy:
	xor eax, eax
	push ebp
	mov ebp, esp
	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]

.moveData:
	lodsb
	stosb
	loop .moveData

	mov esp, ebp
	pop ebp
	ret








