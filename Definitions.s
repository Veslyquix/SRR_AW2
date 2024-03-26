@.include "aw2.s" @ if I find one 

.macro SET_FUNC name, value
	.global \name
	.type   \name, function
	.set    \name, \value
.endm

.macro SET_DATA name, value
	.global \name
	.type   \name, object
	.set    \name, \value
.endm


SET_FUNC __aeabi_idiv, 0x808AAAD
SET_FUNC Div, 0x808AAAD
SET_FUNC Mod, 0x808AAB1
SET_FUNC CpuSet, 0x808AAA9 
SET_FUNC CpuFastSet, 0x808AAA5
SET_FUNC BgAffineSet, 0x808AAA0 

SET_DATA gCh, 0x3004FC2 @ from 0x8078E10 
SET_DATA RandValues, 0x203FFFC
SET_DATA RandBitflags, 0x203FFF8
SET_DATA gGameClock, 0x3004008 @ in 80368D4 

@ GetClassAtt ? 
@0804310C - Atk
@0804317C - Def
@080431EC - Mov
@0804325C - Rng

 