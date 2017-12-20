mov r0, #0
swi 0x6c
MOV r2, r0 @r2 stores n
MOV r3, #0
MOV r4, #1
MOV r6, #0
MOV r7, #10
MOV r8, #0
loop:
	CMP r2, #1	
	BLE loop_exit
	ADD r4, r4, r3
	SUB r3, r4, r3
	SUB r4, r4, r3
	ADD r4, r4, r3
	SUB r2, r2, #1
	CMP r2, #1	
B loop
loop_exit:
	mov r4, r3
	loop1_5:
		CMP r4, #0
		BLE loop1_5exit 
	mov r2, #0 @quotient
	mov r5, #0 @remainder
	loop2:
		SUB r4, r4, #10
		CMP r4, #0
		BLT loop2_exit
		ADD r2, r2, #1
	B loop2	
	loop2_exit:
		add r5, r4, #10
		mul r8, r6, r7
		mov r6, r8
		add r6, r6, r5
		mov r4, r2
		
	B loop1_5
	loop1_5exit:
		mov r0, #1
		mov r1, r6
		swi 0x6b
		swi 0x11	
		