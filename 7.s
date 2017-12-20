mov r0, #0
swi 0x6c
mov r1, r0 @stores number of elements
mov r2, #0 @stores sum
mov r3, #0 @counter
loop:
	mov r0, #0
	swi 0x6c
	ADD r2, r2, r0
	ADD r3, r3, #1 @counter increment
	CMP r3, r1  
	BGE loop_exit
B loop
loop_exit:
	MOV r0, #1
	MOV r1, r2
	swi 0x6b
	swi 0x11