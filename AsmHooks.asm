.thumb 

@ 80266DC 
@ 8085244
@80856a0
@ 30033EC - current phase (1, 2, 3, or 4) 

.global DefaultPowCoPageHook
.type DefaultPowCoPageHook, %function
DefaultPowCoPageHook:

@ldr r3, =0x3005940 @ page 
@mov r1, #4 
@strb r1, [r3] @ default to page 4 

add r0, #0x64 
mov r1, #1 
strh r1, [r0] @ redraw ? 

mov r1, #0 
ldr r0, =0x30033EC 
ldrh r0, [r0] 
mov r5, r4 



bx lr 
.ltorg 

.global ModNum 
.type ModNum, %function 
ModNum: 
swi 6 
mov r0, r1 
bx lr 

.global GetClassAtt
.type GetClassAtt, %function 
GetClassAtt: 
push {r4-r5, lr} 
sub sp, #8 
str r0, [sp]@ CO id 
str r2, [sp, #4]@ offset 

ldr r0, =0x85D3E2C 
add r3, r0 
ldr r7, [r3] 
add r0, r2 
ldr r3, =0x80432E0 
mov lr, r3 
.short 0xf800 
lsl r0, #2 
add r0, r5
add r0, r4 
ldr r1, =0x85d3e2c 
add r0, r1 
ldr r1, [r0] 
ldr r0, =0x3003fc0 
ldrb r0, [r0, #8]
cmp r0, #0 
bne AttCont
mov r0, #0 
b ExitAtt 


AttCont:
ldrb r0, [r6, #0x18]
mov r2, #0 
cmp r0, #0 
beq ReturnDefaultAtt
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
ldr r1, [sp]
ldr r2, [sp, #4]
mov r3, r5 
bl HashPow @ must be multiples of 10  
b ExitAtt 
ReturnDefaultAtt: 

ldsh r0, [r7, r2] 
ExitAtt: 
add sp, #8
pop {r4-r5} 
pop {r3} 
bx r3 
.ltorg

.global GetClassDef
.type GetClassDef, %function 
GetClassDef: 
push {r4-r5, lr} 
sub sp, #8 
str r0, [sp]@ CO id 
str r2, [sp, #4]@ offset 

ldr r0, =0x85D3E2C 
add r3, r0 
ldr r7, [r3] 
add r0, r2 
ldr r3, =0x80432E0 
mov lr, r3 
.short 0xf800 
lsl r0, #2 
add r0, r5
add r0, r4 
ldr r1, =0x85d3e2c 
add r0, r1 
ldr r1, [r0] 
ldr r0, =0x3003fc0 
ldrb r0, [r0, #8]
cmp r0, #0 
bne DefCont 
mov r0, #0 
b ExitDef 


DefCont:
ldrb r0, [r6, #0x18]
mov r2, #2
cmp r0, #0 
beq ReturnDefaultDef
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
ldr r1, [sp]
ldr r2, [sp, #4]
mov r3, r5 
bl HashDef @ must be multiples of 10  
b ExitDef 
ReturnDefaultDef: 

ldsh r0, [r7, r2] 
ExitDef: 
add sp, #8
pop {r4-r5} 
pop {r3} 
bx r3 
.ltorg

.global GetClassMov
.type GetClassMov, %function 
GetClassMov: 
push {r4-r5, lr} 
sub sp, #8 
str r0, [sp]@ CO id 
str r2, [sp, #4]@ offset 

ldr r0, =0x85D3E2C 
add r3, r0 
ldr r7, [r3] 
add r0, r2 
ldr r3, =0x80432E0 
mov lr, r3 
.short 0xf800 
lsl r0, #2 
add r0, r5
add r0, r4 
ldr r1, =0x85d3e2c 
add r0, r1 
ldr r1, [r0] 
ldr r0, =0x3003fc0 
ldrb r0, [r0, #8]
cmp r0, #0 
bne MovCont 
mov r0, #0 
b ExitMov

MovCont:
ldrb r0, [r6, #0x18]
mov r2, #4
cmp r0, #0 
beq ReturnDefaultMov
ldsh r0, [r7, r2] 
ldsh r1, [r1, r2] 
add r0, r1 
ldr r1, [sp]
ldr r2, [sp, #4]
mov r3, r5 
bl HashMov  
b ExitMov 
ReturnDefaultMov: 

ldsh r0, [r7, r2] 
ExitMov: 
add sp, #8
pop {r4-r5} 
pop {r3} 
bx r3 
.ltorg


.global GetClassRange
.type GetClassRange, %function 
GetClassRange: 
push {r4-r5, lr} 
sub sp, #8 
str r0, [sp]@ CO id 
str r2, [sp, #4]@ offset 

ldr r0, =0x85D3E2C 
add r3, r0 
ldr r7, [r3] 
add r0, r2 
ldr r3, =0x80432E0 
mov lr, r3 
.short 0xf800 
lsl r0, #2 
add r0, r5
add r0, r4 
ldr r1, =0x85d3e2c 
add r0, r1 
ldr r1, [r0] 
ldr r0, =0x3003fc0 
ldrb r0, [r0, #8]
cmp r0, #0 
bne RangeCont 
mov r0, #0 
b ExitRange

RangeCont:
ldrb r0, [r6, #0x18]
mov r2, #6
mov r3, #4 @ for mov 
cmp r0, #0 
beq ReturnDefaultRange
ldsh r0, [r7, r2] 
ldsh r2, [r7, r3] 
ldsh r3, [r7, r3] 
add r3, r2 

ldsh r1, [r1, r2] 
add r0, r1 


ldr r1, [sp]
ldr r2, [sp, #4]
str r5, [sp] 
bl HashRange  
b ExitRange
ReturnDefaultRange: 

ldsh r0, [r7, r2] 
ExitRange: 
add sp, #8
pop {r4-r5} 
pop {r3} 
bx r3 
.ltorg

@ 21674 
.global ReplaceMapHook 
.type ReplaceMapHook, %function 
ReplaceMapHook: 
push {lr} 
mov r0, #0 
strh r0, [r4, #0x4] 
strh r0, [r4, #0x6] 
strh r0, [r4, #0x8] 
strh r0, [r4, #0xA] 
strh r0, [r4, #0xC] 
strh r0, [r4, #0xE] 
strh r0, [r4, #0x10] 

mov r0, r2 @ map in ram 
mov r1, r4 
bl GenerateMap 

mov r0, #0 
pop {r1} 
bx r1 
.ltorg 
 
 
