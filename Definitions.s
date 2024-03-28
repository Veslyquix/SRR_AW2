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
SET_FUNC BgAffineSet, 0x808AAA1 
SET_FUNC LoadDesignRoomName, 0x803CC85

SET_DATA gCh, 0x3004FC2 @ from 0x8078E10 
@SET_DATA RandValues, 0x203FFFC
SET_DATA RandValues, 0x200B204
SET_DATA DesignRoom1Name, 0x200B204
SET_DATA RandBitflags, 0x203FFF8
SET_DATA gGameClock, 0x3004008 @ in 80368D4 

 

SET_DATA gPlayer0, 0x2023284
SET_DATA gPlayer1, 0x20232C0
SET_DATA gPlayer2, 0x20232FC
SET_DATA gPlayer3, 0x2023338
SET_DATA gPlayer4, 0x2023374


@ GetClassAttBonus 
@0804310C - Atk
@0804317C - Def
@080431EC - Mov
@0804325C - Rng


@ 200B09C 
@ 200B204 
@ 20004B2 
@ E008000


