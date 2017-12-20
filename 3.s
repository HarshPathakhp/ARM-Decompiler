
mov r0,#0
swi 0x6c
mov r1,r0 @ no of elements
mov r2,#0  @counter
mov r3,#1  @ stores fact
loop:
	mov r0,#0
	swi 0x6c
	cmp r0,r1 
	MUL r3,r0,r3

	add r2,r2,#1
	sub r1,r1,#1
	cmp r1,r2
	beq loop3

	cmp r1,#0
	beq loop_exit
	b loop

loop1:
	add r3,r3,#1
	b loop
loop3:
	add r3,r3,#2
	b loop
loop_exit:
	MOV r0, #1
	MOV r1, r3
	swi 0x6b
swi 0x11


