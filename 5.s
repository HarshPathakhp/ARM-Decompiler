b main
fib2:
	cmp r2, #5
	bne gr8
		mov pc, lr
	gr8:
		add r1,r1,r2
		add r2,r2,#1
		blx fib2
		mov pc, lr
main:
	mov r1,#0
	mov r2,#1
	blx fib2
	mov r0, #1
	swi 0x6b 
	swi 0x11



