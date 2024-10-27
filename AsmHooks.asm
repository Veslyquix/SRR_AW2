.thumb 

.macro blh to, reg=r3
  ldr \reg, =\to
  mov lr, \reg
  .short 0xf800
.endm

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
mov r0, r2 
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
@beq ReturnDefaultAtt
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
mov r0, r2 
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
@beq ReturnDefaultDef
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
mov r0, r2 
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
@beq ReturnDefaultMov
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
mov r0, r2 
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
cmp r0, #0 
@beq ReturnDefaultRange

mov r4, #4 @ for mov 

ldsh r2, [r7, r4] 
ldsh r3, [r1, r4] 
add r3, r2 

mov r4, #6 @ range 

ldsh r0, [r7, r4] 
ldsh r1, [r1, r4] 
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
.global MapSizeHook 
.type MapSizeHook, %function 
MapSizeHook: 
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
mov r1, r4 @ ch header 
mov r2, r5 @ chID 
bl SetMapSize 
mov r0, #0 
pop {r1} 
bx r1 
.ltorg 


.global ReplaceMapHook 
.type ReplaceMapHook, %function 
ReplaceMapHook: 
push {lr} 
strb r0, [r1] 
ldr r3, =0x80215FC 
mov lr, r3 
.short 0xf800 @ blh 
ldr r0, =0x3003F68 
ldr r0, [r0] 
ldr r1, =0x8499590 
ldr r1, [r1] @ 0x201E450 
mov r2, r5 @ chID 
bl GenerateMap 

mov r1, #0 
ldr r0, [r6] 
ldrh r0, [r0, #2] 
pop {r3} 
bx r3 
.ltorg 

.global GetPreEventPointerHook
.type GetPreEventPointerHook, %function 
GetPreEventPointerHook: 
push {lr} 
add r0, r1 
ldr r4, [r0] 
add r2, #0x2c 
add r1, r2 
ldr r1, [r1] 

push {r0-r1} 
bl ShouldMapBeRandomized
mov r2, r0 
pop {r0-r1} 
cmp r2, #0 
beq GetPreEventPointerHook_Vanilla 
mov r4, #0 

GetPreEventPointerHook_Vanilla: 
cmp r4, #0 
pop {r3} 
bx r3 
.ltorg 

.global GPE_2 @ 74584 
.type GPE_2, %function 
GPE_2: 
push {lr} 
ldr r0, =0x3003FC0 
ldrb r0, [r0, #2] 
blh 0x8035000 
ldr r0, [r0, #4] 
push {r0} 
bl ShouldMapBeRandomized
pop {r1} 
cmp r0, #0
beq GPE_2_Vanilla
mov r1, #0  

GPE_2_Vanilla: 
mov r0, r1 
pop {r1} 
bx r1 
.ltorg 

.global UnitPointerHook 
.type UnitPointerHook, %function 
UnitPointerHook: 
push {lr} 
ldrb r3, [r0, #2] 
mov r0, #0x5c 
mul r0, r3 
add r1, r0 
add r2, #0x34 
add r1, r2 
ldr r0, [r1] 
push {r0} 
bl ShouldMapBeRandomized 
pop {r1} 
cmp r0, #0 
beq UnitPointerHook_Vanilla 
mov r1, #0 
UnitPointerHook_Vanilla: 
mov r0, r1 
pop {r3} 
bx r3 
.ltorg 

.global GetNumberOfDaysHook
.type GetNumberOfDaysHook, %function 
GetNumberOfDaysHook: @ copy of 43630 
push {r4-r5, lr} 
mov r4, r0 
mov r5, r1 

bl ShouldMapBeRandomized 
cmp r0, #0 
beq VanillaDays 
mov r0, #0 
b ExitDays 
VanillaDays: 
mov r0, r4 
mov r1, r5 
ldr r2, =0x8043650 
ldr r2, [r2]  
ldr r3, =0x8043654 
ldr r3, [r3]  
ldrb r1, [r3, #2] 
mov r0, #0x5c 
mul r0, r1 
add r1, r0, r2 
ldrh r0, [r1, #0x24] 
cmp r0, #0 
bne ExitDays 
mov r1, r3 
add r1, #0x30 
ldrb r0, [r1] 

ExitDays: 
pop {r4-r5} 
pop {r1} 
bx r1 
.ltorg 

@ 618f8 

@0x618a8 callHackNew short 0x46c0 

.global EnsureFactoryPointer 
.type EnsureFactoryPointer, %function 
EnsureFactoryPointer: 
push {lr} 
add r0, r2 
ldr r0, [r0] 
cmp r0, #0 
bne StoreFactoryPointerVanilla 
ldr r0, =GenericFactoryUnits 
StoreFactoryPointerVanilla: 
str r0, [r5] 
blh 0x8061cdc 
blh 0x8061cf8 
pop {r1} 
bx r1 
.ltorg 

.global ToggleMusicWinCh
.type ToggleMusicWinCh, %function 
ToggleMusicWinCh: 
push {lr} 
blh WinCh_P1
blh WinCh_P2


ldr r3, =0x802CE20 
ldr r3, [r3] 
ldr r1, [r3] 
ldrb r2, [r1, #0xC] 
mov r0, #1 
sub r0, r2 
strb r0, [r1, #0xC] 
ldrb r0, [r1, #0xC] 
mov r4, r3 
pop {r3} 
bx r3 
.ltorg 

.global EnsureFactoryPointer2 
.type EnsureFactoryPointer2, %function 
EnsureFactoryPointer2: 
push {r4-r5, lr} 
sub sp, #0x60 
ldr r4, [r6]
ldr r5, [r4] 
cmp r5, #0 
bne UseVanillaFactoryPointer



mov r1, sp 
ldr r2, =GenericFactoryUnits 
mov r3, #0 
DefaultFactoryUnitsLoop:
ldr r0, [r2, r3] 
str r0, [r1, r3] 
add r3, #4 
cmp r3, #0x60  
blt DefaultFactoryUnitsLoop 
blh 0x803866c @ difficulty 
mov r1, sp 
str r1, [r4] 
bl RandomizeFactoryUnits 
UseVanillaFactoryPointer: 
blh 0x80607e8
str r5, [r4] @ restore to normal afterwards 
add sp, #0x60 
pop {r4-r5} 
pop {r0} 
bx r0 
.ltorg 



.global GetCampaignHeader
.type GetCampaignHeader, %function 
GetCampaignHeader: @ see 0x807733a 
push {lr} 
blh 0x803866c @ difficulty & checks if we're actually in campaign 
lsl r3, r0, #2 @ either 4 or 0  
ldr r2, =0x80773a4 
ldr r2, [r2] @ldr r2, =0x81CC5A0 @ r5=81CC5A0
ldr r2, [r2] 
ldr r0, [r2, #0xC] @ 0202FE08 current mission highlighted
ldr r2, =0x80773a0 
ldr r2, [r2] @ldr r3, =0x8615194 
lsl r1, r0, #1 
add r1, r0 
lsl r1, #4 
add r0, r1, r2
pop {r1} 
bx r1 
.ltorg 

.global GetFactoryUnitsPointer 
.type GetFactoryUnitsPointer, %function 
GetFactoryUnitsPointer: @ 
push {lr} 
bl GetCampaignHeader 
ldr r0, [r0, #0x24] 
pop {r1} @ [0x8615194+0x24]?
bx r1 
.ltorg 
@mov r3, #0 
@ldsh r2, [r0, r3] @ gives 0x8A 
@ldr r3, =0x85CC7A0 
@mov r0, #0x5C 
@mul r0, r2 
@add r2, r0, r3 
@ldr r0, [r2, #0x24] 


@ [0x85CA938+0x24]? 8615194+0x24 

.global GetNumberOfPlayers 
.type GetNumberOfPlayers, %function 
GetNumberOfPlayers: @ see 0x807733a 
push {r4-r5, lr} 
@ldr r4, =0x8615194 
@ldr r5, =0x81cc5a0 
@ldr r2, [r5] 
@ldr r0, [r2, #0xC] 
@lsl r1, r0, #1 
@add r1, r0 
@lsl r1, #4 
@add r0, r1, r4 
@mov r3, #0 
@ldsh r2, [r0, r3] 

ldr r2, =0x3003FC2 
ldrb r2, [r2] 
ldr r3, =0x85c77a0 
mov r0, #0x5C 
mul r0, r2 
add r2, r0, r3 
ldrb r0, [r2, #0x18] 
pop {r4-r5} 
pop {r1} 
bx r1 
.ltorg 


.global BitSet_UnlockAll
.type BitSet_UnlockAll, %function 
BitSet_UnlockAll:
	push	{r14}
	ldr	r0,=0x17F	@Campaign/Game Modes
	ldr	r1,=0x02028030
	strh	r0,[r1]
	
	mov	r0,#0x1		@Map Cases
	neg	r0,r0
	add	r1,#0x10
	strb	r0,[r1]
	
	add	r1,#2
	mov	r2,#0		@Maps
Map_Loop_Start:	
	add	r2,#0x1
	strb	r0,[r1,r2]
	cmp	r2,#0x11
	blt	Map_Loop_Start
	
	ldr	r0,=0xFFFF	@COs
	add	r1,#0x18
	strb	r0,[r1]
	strb	r0,[r1,#0x1]
	strb	r0,[r1,#0x2]
	
	ldr	r0,=0xFFFF	@Colour Edits
	add	r1,#0x3
	strb	r0,[r1]
	strb	r0,[r1,#0x1]
	strb	r0,[r1,#0x2]
	
	pop	{r1}
	bx	r1
	.ltorg
 
.global memset 
.type memset, %function 
memset: 
@ void *memset(void *dest, int val, u32 len)
push {r0} 
sub sp, #4 

mov r3, #0 
str r3, [sp] 

lsr r2, #1 @ in SHORT units, not bytes 
mov r3, #1 
lsl r3, #24 @ fill, not copy 
orr r2, r3 

lsr r1, r0, #1 
lsl r1, #1 
mov r0, sp 
swi 0xB 

add sp, #4 
pop {r0} 
bx lr 
.ltorg 

.global memcpy 
.type memcpy, %function 
memcpy: 
@ void *memcpy(void *src, void *dest, u32 len)
push {r0} 
lsr r0, #1 
lsl r0, #1 
lsr r1, #1 
lsl r1, #1 
lsr r2, #1 @ in SHORT units, not bytes 
swi 0xB 

pop {r0} 
bx lr 
.ltorg 

 
 
