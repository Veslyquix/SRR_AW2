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

SET_FUNC MakeRiver, 0x8009F11 
SET_FUNC MakeMountain, 0x800b049 
SET_FUNC MakeForest, 0x800CF29
    SET_FUNC MakeForestSimple, 0x8007d71 
SET_FUNC MakeRoad, 0x800f4e1 
    SET_FUNC MakeRoadSimple, 0x80080f9 
    SET_FUNC GetMostFittingTile, 0x8001705 

SET_FUNC MakeProperty, 0x800C455 @ r0 x r1 y r2 as _City  
SET_FUNC EnsureValidTile, 0x8008BB9 @ might delete surrounding tiles or something 
SET_FUNC MakeProperty2, 0x8008A8D @ r0 as 0-4, r1 x r2 y 

SET_FUNC MakeSeaSafest 0x8007CA1 
SET_FUNC MakeSeaSafe, 0x8007c05 @ ??? 
SET_FUNC MakeSea, 0x8007BA5 
@ 40c8 
    @ BA9C 
        @ MakeSeaSafest 

@SET_FUNC MakeHQ, 0x800C455 same as MakeProperty 
SET_FUNC MakeBridge, 0x8008F6D 
SET_FUNC MakeShoal, 0x8007BA5 @ Called by MakeSeaSafe
SET_FUNC MakePipe, 0x8010D29 
SET_FUNC MakeSeam, 0x8010D81 
SET_FUNC MakeReefSafe, 0x800BF79 

SET_DATA mapTileData, 0x201EE72
SET_DATA terrainTileData, 0x201E450+0x1432  
SET_DATA currentTileData, 0x201E450+0x417a

@[201f882]?
@ [201EEE4]! @ 6x 3y tile 
@ [201F8C4]! @ 6x 3y terrain 

SET_FUNC MakeTile, 0x80085E1 @ 
SET_FUNC MakeTile2, 0x8007F15 
SET_FUNC MakeTile3, 0x800CEF9 
SET_FUNC MakeTileSimple, 0x8001159 @ r0 x r1 y r2 tile 



@ 0 forest, 1 plain, 

@ store tile into 0x200B000 
@ coord x into 0x200b008, y into 0x200b00a 




SET_FUNC SetSelectedTile, 0x8000CCD @ [200b02a]!! 

SET_DATA Unk_200B06A, 0x200B06A @ byte stored to after making a tile 

SET_DATA Unk_200B000, 0x200B000 
SET_DATA Unk_200B007, 0x200B007 
SET_DATA SelectedTileX, 0x200B008 
SET_DATA SelectedTileY, 0x200B00A 
SET_DATA Surplus, 0x200B012 
SET_DATA SelectedTile, 0x200B02a @ press B in design room on tile 
SET_DATA Unk_200B036, 0x200B036 @ 
SET_DATA SelectedTile_3A, 0x200B03A 
SET_DATA PreviousTile_3C, 0x200B03C 
SET_DATA gActiveMap, 0x200B0B0 


SET_FUNC Rand, 0x80129E1 
SET_FUNC __aeabi_idiv, 0x808AAAD
SET_FUNC Div, 0x808AAAD
SET_FUNC Mod, 0x808AAB1
SET_FUNC CpuSet, 0x808AAA9 
SET_FUNC CpuFastSet, 0x808AAA5
SET_FUNC BgAffineSet, 0x808AAA1 
SET_FUNC LoadDesignRoomName, 0x803CC85

SET_DATA gCh, 0x3003FC2 @ from 0x8078E10 
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


