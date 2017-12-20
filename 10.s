B fact1
fnstarts:
loop:  
	  CMP r1,r0    @i <= N
      BGT exit
      MUL r2,r1,r2
      ADD r1,r1,#1
      B loop
exit: 
	MOV   pc,lr      
fact1:
	MOV r0,#10 
	MOV   r2,#1    
    MOV   r1,#1
    BL fnstarts
    MOV r2,#3
