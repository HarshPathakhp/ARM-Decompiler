b main
fn:
add r1,r1,#1
cmp r1,#10
bge skip
	mov r0,#0
	blx fn
skip:
mov pc,lr
main:
mov r0,#0
swi 0x6c
mov r1,r0
blx fn
mov r0,#1
swi 0x6b
