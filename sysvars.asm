;Memory addresses
os_SystemVars	equ	0x00000500

;DB starting at offeset 0, increment by 1
screen_text_x	equ	os_SystemVars
screen_text_y	equ	os_SystemVars + 1
text_color	equ	os_SystemVars + 2

