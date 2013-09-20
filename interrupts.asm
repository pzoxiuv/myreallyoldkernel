section .text:
	global isr0
	global isr1
	global isr2
	global isr3
	global isr4
	global isr5
	global isr6
	global isr7
	global isr8
	global isr9
	global isr10
	global isr11
	global isr12
	global isr13
	global isr14
	global isr15
	global isr16
	global isr17
	global isr18
	global irq15
	global irq1
	global irq0
	global irq
	global sys
	extern syscl
	extern interrupt
	extern keyboard
	extern drive
	extern timerTick

sys:
	cli
	push eax
	mov al, 0x20
	out 0x20, al
	out 0xA0, al
	pop eax
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	;push dword [esp+24]
	call syscl
	;pop ebx
	pop ebx
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	sti
	iret

isr0:
	cli
	push byte 0			;error code
	push byte 0			;isr num
	jmp isr

isr1:
	cli
	push byte 0			;error code
	push byte 1			;isr num
	jmp isr

isr2:
	cli
	push byte 0			;error code
	push byte 2			;isr num
	jmp isr

isr3:
	cli
	push byte 0			;error code
	push byte 3			;isr num
	jmp isr

isr4:
	cli
	push byte 0			;error code
	push byte 4			;isr num
	jmp isr

isr5:
	cli
	push byte 0			;error code
	push byte 5			;isr num
	jmp isr

isr6:
	cli
	push byte 0			;error code
	push byte 6			;isr num
	jmp isr

isr7:
	cli
	push byte 0			;error code
	push byte 7			;isr num
	jmp isr

isr8:
	cli
	push byte 8			;isr num
	jmp isr

isr9:
	cli
	push byte 0			;error code
	push byte 9			;isr num
	jmp isr

isr10:
	cli
	push byte 10			;isr num
	jmp isr

isr11:
	cli
	push byte 11			;isr num
	jmp isr

isr12:
	cli
	push byte 12			;isr num
	jmp isr

isr13:
	cli
	push byte 13			;isr num
	jmp isr

isr14:
	cli
	xchg bx, bx
	;pop eax
	;pop ebx
	;hlt
	push byte 14			;isr num
	jmp isr

isr15:
	cli
	push byte 0			;error code
	push byte 15			;isr num
	jmp isr

isr16:
	cli
	push byte 0			;error code
	push byte 16			;isr num
	jmp isr

isr17:
	cli
	push byte 0			;error code
	push byte 17			;isr num
	jmp isr

isr18:
	cli
	push byte 0			;error code
	push byte 18			;isr num
	jmp isr

;isr:
;	pusha
;	mov ax, ds
;	push eax
;	mov ax, 0x10
;	mov ds, ax
;	mov es, ax
;	mov fs, ax
;	mov gs, ax
;	call interrupt
;	pop ax
;	mov ds, ax
;	mov es, ax
;	mov fs, ax
;	mov gs, ax
;	popa
;	add esp, 8
;	sti
;	iret

isr:
	xor eax, eax
	pop eax				;pop off isr num
	;pop bx				;pop off error code
	mov ebx, [esp]
	mov ecx, [esp+4]
	mov edx, cr2
	;jmp $
	;hlt
	pusha
	push ds
	push es
	push fs
	push gs
	push edx
	push ecx
	push ebx
	push eax
	call interrupt
	pop eax	
	pop ebx
	pop ecx
	pop edx
	pop gs
	pop fs
	pop es
	pop ds
	popa
	sti
	iret

irq:
	cli
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;call drive
	pop gs
	pop fs
	pop es
	pop ds
	popa
	mov al, 0x20
	out 0x20, al
	out 0xA0, al
	sti
	iret
	
irq0:
	cli
	;pushad
	;push ds
	;push es
	;push fs
	;push gs
	;mov ax, 0x10
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax
	push eax
	push ebx
	push ecx
	push edx
	push edi
	push esi
	push ebp
	mov ecx, [esp+(4*7)]			;grab eip off stack
	mov edx, [esp+8+(4*7)]			;and grab eflags
	mov ebx, esp
	add ebx, 12+(4*7)			;eip, cs, eflags were pushed on when this ISR was called, so the old esp is currentESP+12+(4*7) (for the 7 general reg's pushed on)
	push ecx
	push ebx
	push edx
	mov al, 0x20
	out 0x20, al
	mov eax, timerTick
	call eax
	pop eax
	pop eax
	pop eax
	pop ecx
	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	;cmp eax, 0x12345
	;jne .done
	;mov [esp], ebx
	;cli
	;jmp $
	;mov [esp+16], ebx
	;pop gs
	;pop fs
	;pop es
	;pop ds
	;popa
	;mov al, 0x20
	;out 0x20, al
	;popad
	sti
	iret

irq1:
	cli
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call keyboard
	pop gs
	pop fs
	pop es
	pop ds
	popa
	mov al, 0x20
	out 0x20, al
	sti
	iret

irq15:
	cli
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call drive
	pop gs
	pop fs
	pop es
	pop ds
	mov al, 0x20
	out 0xA0, al
	out 0x20, al
	popa
	sti
	iret


