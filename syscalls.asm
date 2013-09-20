%include "sysvars.asm"

section .text:
	global malloc
	global free
	global clearScreen
	global kprintf
	global updateCursor
	global strcmp
	global strlen
	global stoi
	global itos
	global subStr
	global appendStrs
	global printf

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;MEMORY.ASM;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;in: number of requested bytes (in eax)
;returns: address of beginning of block

malloc:
	push ebx	
	push ecx
	push edx
	
	mov ebx, firstMemAddr

.findFreeBlock:
	cmp [ebx], byte 0
	je .foundFreeBlock
	add ebx, 4		;not free, get size of block and advance to next block
	add ebx, [ebx]
	jmp .findFreeBlock
	
.foundFreeBlock:
	add ebx, 4		;advance to size header
	cmp eax, [ebx]		;compare requested size to size of block
	jle .done1		;if requested size is less than size of block we're done
	add ebx, [ebx]		;otherwise, move to next block
	inc ebx			;move to next header
	jmp .findFreeBlock	;and keep looking for a free block

.done1:
	inc ebx			;move to first address of memory block (after the headers)
	mov ecx, eax		;save size requested
	mov eax, ebx		;save address in eax for returning
	sub ebx, 5
	mov [ebx], byte 1	;set use header to 1 (in use)
	add ebx, 4		;get back to size header
	push dword [ebx]	;store size of free block on stack
	mov [ebx], ecx		;set new size
	pop ecx			;pop size of free block to stack
	add ebx, ecx		;add size of block to address of start of data
	cmp ebx, totalMemAmount	;if address+size = max mem, we need to put a new header in
	jge .addNewHeader
	jmp .done2	

.addNewHeader:
	mov ebx, eax
	add ebx, [eax-1]	;get to where next header should be
	mov [ebx], byte 0	;set use to unused
	add ebx, 4		;get to size header
	mov ecx, totalMemAmount	;get max size
	sub ecx, ebx		
	mov [ebx], ecx
	mov ecx, 1

.done2:		
	
	pop edx
	pop edx
	pop edx
	ret

;in: address of start of DATA (not header!) of block to be free'd
;returns: 0

free:
	sub eax, 5		;get to size header
	;mov ebx, eax
	;add ebx, [ebx]		;add size of block to address of beginning of size header
	;add ebx, 4		;and add four for the size header
	mov [eax], byte 0	;set use header for block to 0 (unused)
	mov eax, 0		;return 0 (not like we check anyway...)
	ret
	
	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;SCREEN.ASM;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

clearScreen:
	push edi
	push ecx
	push eax

	mov edi, 0xB8000
	mov ecx, 2000
	mov ax, 0x0720		;07 is attribute, 20 is space char
	rep stosw
	
	pop eax
	pop ecx
	pop edi
	ret

kprintf:
	push esi
	push eax
	push edi
	push ebx
	
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
	
	mov ah, 0x07
.print:
	lodsb
	cmp al, 0
	je .done
	cmp al, 10
	je .newLine
	cmp al, 9
	je .tab
	stosw
	inc ebx
	jmp .print
.newLine:
	mov ecx, screen_text_y		;incriment line num
	inc byte [ecx]	
	add edi, 160			;move to next line in video mem
	shl ebx, 1			;ebx contains real x value (orig x+chars printed)
	sub edi, ebx			;multiply it by two and subtract from edi to get 
	xor ebx, ebx			;back to beginning of next line
	mov [screen_text_x], bl		;reset x to 0	
	jmp .print
.tab:
	mov ecx, screen_text_x
	add [ecx], byte 8
	add edi, 16
	jmp .print
.done:
	add [screen_text_x], ebx
	call updateCursor

	pop ebx
	pop edi
	pop eax
	pop esi
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;STRING.ASM;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;compares two strings alphabetically
;args: str1, str2
;returns: 0 if equal, 1 otherwise

strcmp:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]	;str1
	mov ebx, [ebp+12]	;str2
	xor ecx, ecx
	xor edx, edx
	xor edi, edi
.loopStrings:
	movzx ecx, byte [eax]	;only want one byte at a time
	movzx edx, byte [ebx]
	cmp ecx, edx		
	jne .notEqual		
	cmp ecx, byte 0		;end of first string, must be end of second
	je .done		;as well
	inc eax			
	inc ebx			
	jmp .loopStrings	
.notEqual:			;not equal, set edi to 1
	mov edi, 1		;if this doesn't happen, edi remains 0
.done:				
	mov eax, edi		;mov edi into eax for returning
	
	mov esp, ebp		;restore stack and registers
	pop ebp
	ret

;returns the length of a null terminated string
;starts counting from 1 (so "three" will return a length of 5)
;args: *string
;returns: length (as an int)

strlen:
	push ebp
	mov ebp, esp
	mov ebx, [ebp+8]	;string
	xor eax, eax
.getLen:			;loop through string, incrimenting counter
	inc eax			;until we reach the null terminator
	inc ebx
	cmp [ebx], byte 0
	jne .getLen

	mov esp, ebp		;restore stack
	pop ebp
	ret


;Converts string to int
;args: *string
;returns: int placed in eax

stoi:
	push ebp
	mov ebp, esp
	mov esi, [ebp+8]		;*string
	xor ebx, ebx
	xor eax, eax
	xor edx, edx
	movzx ebx, byte [esi]		;load first byte, check if it's a -
	cmp ebx, '-'			
	jne .convertToInt		;no, not negative, start converting!
	mov edx, 1			;1 means negative
	inc esi				;move to first actual number

.convertToInt:
	movzx ebx, byte [esi]		;load next number (byte of string)
	sub ebx, '0'			;convert to num
	cmp ebx, 9			;check if it's above 9 (unsigned)
	ja .setSign			;yes, not a number, get outta here
	imul eax, 10			;multiply current number by 10
	add eax, ebx			;and add next number to it
	inc esi				;move to next byte of string...
	jmp .convertToInt		;and continue
	
.setSign:				;done converting number, check
	cmp edx, 1			;if it was signed
	jne .done	
	neg eax				;it was, make number negative

.done:
	mov esp, ebp			;restore stack, eax already has num
	pop ebp
	ret

;Converts int to string
;args: int
;returns: *string in eax

itos:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]	;int
	xor ecx, ecx
	mov ebx, 10		;divisor
	mov esi, itosBuf
	test eax, eax
	jns .start		;test for negative
	mov [esi], byte '-'	;was negative, put a minus sign in front
	inc esi			;of string
	neg eax			;make number positive

.start:
	inc ecx		;keep track of how many digits we have
	xor edx, edx
	div ebx		;divides eax by 10
	add dl, '0'	;add'0'to lower byte of remainder to convert to char
	push edx	;pushes remainder. if num was 453, 
	cmp eax, 0	;'3', '5', '4' will be pushed on stack in that order
	jne .start

.makeStr:
	pop edx
	mov [esi], edx	;moves pop'd char into next esi value
	inc esi		;move to next pos in esi
	loop .makeStr	

.done: 
	inc esi			;make room for nul terminator
	mov esi, dword 0	;and add it
	mov eax, itosBuf	;put start address of *string in eax
	
	mov esp, ebp		;restore stack
	pop ebp
	ret

;creates substr from index1 (inclusive) to index2 (exclusive) 
;args: [ebp+8] = str
;      [ebp+12] = index1
;      [ebp+16] = index2
;returns: places pointer to substring in eax
;vars: eax = substr, ebx = str
;edi = index1 in str
;ecx = counter, starting at 0
;edx = difference between index1 and index2

subStr:
	push ebp
	mov ebp, esp
	mov ebx, [ebp+8]	;string
	mov edx, [ebp+16]	;index 2
	sub edx, [ebp+12]	;gets difference between index1 and 2
	dec edx			;index2 is exclusive (adjust)
	mov edi, [ebp+12]	;index 1
	xor ecx, ecx
	xor eax, eax
	add ebx, edi		;moves string to index1 position

.getSubStr:
	mov eax, ebx		
	inc ecx
	inc ebx
	cmp ecx, edx		;if ecx = edx, we've traversed the substr
	jne .getSubStr

	inc eax			;make room for null terminator
	sub eax, ecx		;move address back to start of substr
	mov [eax+ecx+1], byte 0	;add null terminator
	
	mov esp, ebp		;restore stack
	pop ebp
	ret

;args: str1, str2
;retruns: *(str1+str2)

appendStrs:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]	;str1
	mov ebx, [ebp+12]	;str2
	xor ecx, ecx
	xor edx, edx

.endStrOne:			;traverse to end of str1
	inc eax			
	inc ecx
	cmp [eax], byte 0
	jne .endStrOne

.addStrTwo:			;add str2 to end of str1
	mov edx, [ebx]
	mov [eax], edx
	inc eax
	inc ebx
	inc ecx
	cmp [ebx], byte 0
	jne .addStrTwo

	mov [eax], byte 0	;add null terminator to end
	sub eax, ecx		;mov address back to start of str1

	mov esp, ebp		;restor stack
	pop ebp
	ret

;Formats and prints a string, inserting other characters, strings, or integers
;at designated spots.
;%s inserts string, %c inserts char, %d or %i inserts int
;\t inserts a horizontal tab, \n inserts a newline
;args: [ebp+8] = initial string
;      [ebp+8+(4*x)] = other args
;returns: 0

printf:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8]	;*str

	mov ecx, printBuf
	dec ebx
	dec ecx
	xor eax, eax
	xor edi, edi

.loopStr:			;loop through string, adding one
	inc ecx			;char at a time to buf (ecx)
	inc ebx
	cmp [ebx], byte '%'	;catch argument
	je .arg
	cmp [ebx], byte 92	;'\', catch special char (newline, tab)
	je .specialChar
	cmp [ebx], byte 0	;end of orig. string
	je .done	
	inc eax
	push dword [ebx]	;normal char, push the char onto stack
	pop dword [ecx]		;and pop it off into ecx
	jmp .loopStr

.specialChar:
	inc eax
	inc ebx
	cmp [ebx], byte 'n'
	je .newLine
	cmp [ebx], byte 't'
	je .tab

.newLine:
	mov [ecx], dword 10
	jmp .loopStr
.tab:
	mov [ecx], dword 9
	jmp .loopStr

.arg:
	mov edx, [ebp+12+(edi*4)];get next argument
	inc edi
	inc eax
	inc ebx
	cmp [ebx], byte 's'	;if next char after % is s, print string
	je .addStr
	cmp [ebx], byte 'c'	;if it's c, just insert a char
	je .addCh
	cmp [ebx], byte 'd'	;if it's d or i, insert an int
	je .addInt
	cmp [ebx], byte 'i'
	je .addInt
	cmp [ebx], byte 'x'	;convert dec to hex
	je .addHex
.addCh:
	inc eax
	mov [ecx], edx
	jmp .loopStr
.addStr:
	inc eax
	push dword [edx]
	pop dword [ecx]	
	inc ecx
	inc edx
	cmp [edx], byte 0
	jne .addStr
	dec ecx
	jmp .loopStr
.addInt:			;convert int to str before inserting
	inc eax
	push eax		;store eax
	push ebx
	push ecx
	push edx
	push esi
	sub esp, 4
	mov [esp], edx		;pass the int
	call itos
	add esp, 4
	pop esi
	pop edx
	pop ecx
	pop ebx
	mov edx, eax		;move new string into edx
	pop eax			;restore counter
	jmp .addStr		;now we just add int-str like normal str

.addHex:
	inc eax
	push eax		;store registers that will be clobbered here
	push ebx
	push ecx
	push edi
	
	mov ecx, 8		;8 nibbles in 32 bit int
	mov eax, edx
	xor edx, edx
	xor ebx, ebx
	mov edi, hexBuf
.convertToHex:	
	rol eax, 4		;rotate next 4 bits around
	mov bl, al		;mov bottom 8 bits from eax to ebx
	and ebx, 0x0F		;and them to get a 4 bit number
	mov dl, [hexDigits+ebx] ;put hex char lower 8 bits of edx
	push eax		;store eax
	mov al, dl		;move edx to eax
	stosb			;put lower byte of eax in address in edi, incriment edi
	pop eax			;restore eax
	loop .convertToHex	

	xor eax, eax		;put null terminator in eax
	stosb			;and put it in address in edi
	mov edx, hexBuf		;put string in edx so it can be printed
	
	pop edi			;restore registers
	pop ecx			
	pop ebx
	pop eax
	jmp .addStr		;edx contains hex string, add it like normal string
	
	
.done:
	mov eax, printBuf

	call kprintf
	mov eax, 0

	mov esp, ebp	;restore stack
	pop ebp
	ret 

printBuf times 180 db 0
itosBuf times 10 db 0
hexBuf	times 10 db 0
hexDigits db "0123456789ABCDEF",0
