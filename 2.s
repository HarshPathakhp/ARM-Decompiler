mov r1,#0
mov r3,#0
loop:
	mov r2,#0
	loop2:
		add r3,r3,#1
		add r2,r2,#1
		cmp r2,#10
	blt loop2
	add r1,r1,#1
	cmp r1,#10
blt loop
mov r0, #1
mov r1,r3
swi 0x6b
exit:


