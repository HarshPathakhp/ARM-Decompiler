MOV r7, #0
MOV r0, #0
SWI 0x6c
MOV r6, r0
MOV r1, #0
MOV r2, #1
CMP r6, #0
BNE LOOP
MOV r6, #20
LOOP:
ADD r1, r2, r1
SUB r2, r1, r2
MOV r0, #1
SWI 0x6b
MOV r0, #' '
SWI 0x0
ADD r7, r7, #1
CMP r7, r6
BNE LOOP
SWI 0x11