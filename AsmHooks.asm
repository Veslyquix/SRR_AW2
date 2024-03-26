.thumb 

.global GetClassAtt
.type GetClassAtt, %function 
GetClassAtt: 
push {lr} 

mov r2, #0 
cmp r0, #0 
beq ReturnDefaultAtt
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
mov r1, r6 @ pointer to co->class ? 
bl HashPow @ must be multiples of 10  
b ExitAtt 
ReturnDefaultAtt: 

ldsh r0, [r7, r2] 
ExitAtt: 
pop {r3} 
bx r3 
.ltorg

.global GetClassDef
.type GetClassDef, %function 
GetClassDef: 
push {lr} 

mov r2, #2
cmp r0, #0 
beq ReturnDefaultDef
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
mov r1, r6 @ pointer to co->class ? 
bl HashDef @ must be multiples of 10  
b ExitDef 
ReturnDefaultDef: 

ldsh r0, [r7, r2] 
ExitDef: 
pop {r3} 
bx r3 
.ltorg 

.global GetClassMov
.type GetClassMov, %function 
GetClassMov: 
push {lr} 

mov r2, #4
cmp r0, #0 
beq ReturnDefaultMov
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
mov r1, r6 @ pointer to co->class ? 
bl HashMov @ must be multiples of 10  
b ExitMov 
ReturnDefaultMov: 

ldsh r0, [r7, r2] 
ExitMov: 
pop {r3} 
bx r3 
.ltorg 


.global GetClassRange
.type GetClassRange, %function 
GetClassRange: 
push {lr} 

mov r2, #6
cmp r0, #0 
beq ReturnDefaultRange
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
mov r1, r6 @ pointer to co->class ? 
bl HashRange @ must be multiples of 10  
b ExitRange
ReturnDefaultRange: 

ldsh r0, [r7, r2] 
ExitRange: 
pop {r3} 
bx r3 
.ltorg 


 
