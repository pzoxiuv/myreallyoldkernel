;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;SCREEN.ASM;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "sysvars.asm"

section .text:
	global clearScreen
	global puts
	global updateCursor
	global backspace
	global setTextColor
	extern kmemcpy

clearScreen:
	push edi
	push ecx
	push eax

	mov edi, (0xB8000)
	mov ecx, 2000
	mov ax, 0x0720		;07 is attribute, 20 is space char
	rep stosw

	pop eax
	pop ecx
	pop edi
	ret

puts:
	pushfd
	cli
	pushad
	
	xor ebx, ebx
	mov esi, eax
	push eax
	xor eax, eax
	mov al, [screen_text_y]
	mov cl, 80
	mul cl
	xor ebx, ebx
	mov bl, [screen_text_x]
	add ax, bx
	shl ax, 1			;multiply by two
	add eax, 0xB8000
	mov edi, eax	
	pop eax
	xor eax, eax
	mov ah, [text_color]

.testEOL:
	lodsb
	cmp al, 0
	je .done
	cmp al, 10
	je .newLine
	cmp al, 9
	je .tab
	test ebx, ebx
	jz .print
	push eax
	mov eax, edi
	sub eax, 0xB8000
	;hlt
	xor edx, edx	
	mov ecx, 160
	div ecx
	;hlt
	pop eax
	test edx, edx
	jz .newLine
.print:
	stosw
	inc bl
	jmp .testEOL
.newLine:
	xor ecx, ecx
	mov ecx, screen_text_y		
	cmp [ecx], byte 24
	je .scrollScreen		;if we're past the end of the screen, scroll everything up one line
	inc byte [ecx]			;incriment line num	
	add edi, 160			;move to next line in video mem
	shl ebx, 1			;ebx contains real x value (orig x+chars printed)
	sub edi, ebx			;multiply it by two and subtract from edi to get 
	xor ebx, ebx			;back to beginning of next line
	mov [screen_text_x], bl		;reset x to 0	
	jmp .testEOL
.scrollScreen:
	push eax			;save eax (string) on the stack
	push esi
	sub esp, 12
	mov [esp], dword 0xB8000		;destination
	mov [esp+4], dword (0xB8000+160)	;source (memory location of second line)
	mov [esp+8], dword 3840		;160*24 - we want to move 24 lines upward
	call kmemcpy
	add esp, 12
	mov eax, 0x07200720			;clear the last line
	mov edi, (0xB8000 + 24*160)
	mov ecx, 80
	rep stosd
	pop esi
	pop eax
	mov edi, (0xB8000 + 24*160)
	mov bl, 24
	mov [screen_text_y], bl
	xor ebx, ebx
	mov [screen_text_x], bl
	jmp .testEOL
.tab:
	mov ecx, screen_text_x
	add [ecx], byte 8
	add edi, 16
	jmp .testEOL
.done:
	mov [screen_text_x], bl
	call updateCursor

	popad
	popfd
	ret

updateCursor:

	push eax
	push edx	
	push ebx
	
	xor eax, eax
	xor edx, edx
	xor ebx, ebx

	mov bl, [screen_text_y]
	mov bh, [screen_text_x]
	
	mov al, 0x50
	mul bl				
	movzx bx, bh
	add bx, ax
	mov al, 0x0E
	mov ah, bh
	mov dx, 0x03D4
	out dx, ax
	inc ax
	mov ah, bl
	out dx, ax

	pop ebx
	pop edx
	pop eax
		
	ret

backspace:
	xor ebx, ebx
	xor eax, eax
	mov al, [screen_text_y]
	mov cl, 80
	mul cl
	mov bl, [screen_text_x]
	dec bl				;move back to where character is
	add ax, bx
	shl ax, 1			;multiply by two
	add eax, 0xB8000
	mov [eax], byte ' ';
	mov [screen_text_x], bl
	call updateCursor
	ret	

;in: background color, foreground color

setTextColor:
	push ebp
	mov ebp, esp
	mov al, [ebp+8]
	mov [text_color], al
	mov esp, ebp
	pop ebp
	ret
