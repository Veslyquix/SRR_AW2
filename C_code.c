
#include "include/aw2.h" 

struct RandomizerSettings { 
	u16 base : 1; 
	u16 growth : 2; // vanilla, randomized, 0%, 100% 
	u16 caps : 2; // vanilla, randomized, 30 
	u16 class : 1; 
	u16 itemStats : 1; 
	u16 foundItems : 1; 
	u16 shopItems : 1; 
	u16 disp : 1;
}; 

struct RandomizerValues { 
	u32 seed : 31; 
	u32 variance : 1; 
	//u32 bonus : 5; 
}; 

extern struct RandomizerSettings RandBitflags; 
extern struct RandomizerValues RandValues; 
extern u8 gCh; 
extern u32 gGameClock;

// 0x0803CC84 - Loads Design Map 1/2/3 Names into RAM
extern void LoadDesignRoomName(int, int); // 0x803CC84 
void LoadDesignRoom1Name(void) { 
	LoadDesignRoomName(0x200B204, 0x20280C2);
}

int NextSeededRN(u16* currentRN) {
    // This generates a pseudorandom string of 16 bits
    // In other words, a pseudorandom integer that can range from 0 to 65535

    u16 rn = (currentRN[1] << 11) + (currentRN[0] >> 5);

    // Shift state[2] one bit
    currentRN[2] *= 2;

    // "carry" the top bit of state[1] to state[2]
    if (currentRN[1] & 0x8000)
        currentRN[2]++;

    rn ^= currentRN[2];

    // Shifting the whole state 16 bits
    currentRN[2] = currentRN[1];
    currentRN[1] = currentRN[0];
    currentRN[0] = rn;

    return rn;
}

void InitSeededRN(int seed, u16* currentRN) {
    // This table is a collection of 8 possible initial rn state
    // 3 entries will be picked based of which "seed" was given

    u16 initTable[8] = {
        0xA36E,
        0x924E,
        0xB784,
        0x4F67,
        0x8092,
        0x592D,
        0x8E70,
        0xA794
    };

    int mod = Mod(seed, 7);

    currentRN[0] = initTable[(mod++ & 7)];
    currentRN[1] = initTable[(mod++ & 7)];
    currentRN[2] = initTable[(mod & 7)];

    if (Mod(seed, 23) > 0)
        for (mod = Mod(seed,  23); mod != 0; mod--)
            NextSeededRN(currentRN);
}

u16 GetNthRN(int n, int seed) { 
	u16 currentRN[3] = { 0, 0, 0 }; 
	InitSeededRN(seed, currentRN); 
	int result = 0; 
	for (int i = 0; i < n; i++) { 
		result = NextSeededRN(currentRN); 
	}

	return result; 
} 
 
int GetInitialSeed(void) { 
	int result = RandValues.seed;
	if (!result) { 
		result = 0; //TacticianName[1] | (TacticianName[2]<<8) | (TacticianName[3]<<16);
		int clock = gGameClock; 
		result = (GetNthRN(clock, result)<<4) | GetNthRN(clock, result); 
	}
	if (result > 999999) { result &= 0xEFFFF; } 
	return result; 
} 

extern u8 DesignRoom1Name[12];
u16 HashByte_Global(int number, int max, int noise, int offset) {
	if (max==0) return 0;
	offset = Mod(offset, 256); 
	u32 hash = 5381;
	hash = ((hash << 5) + hash) ^ number;
	//hash = ((hash << 5) + hash) ^ *StartTimeSeedRamLabel;
	for (int i = 0; i < 12; ++i){
		if (!DesignRoom1Name[i]) { break; } 
		hash = ((hash << 5) + hash) ^ DesignRoom1Name[i];
	};
	
	u8 seed[3] = { (noise & 0xFF), (noise&0xFF00)>>8, (noise&0xFF0000)>>16 }; 
	for (int i = 0; i < 3; ++i){
		if (!seed[i]) { break; } 
		hash = ((hash << 5) + hash) ^ seed[i];
	};

	hash = GetNthRN(offset + 1, hash); 	
	return Mod((hash & 0x2FFFFFFF), max);
};

u16 HashByte_Ch(int number, int max, int noise, int offset){
	noise += (gCh << 8); 
	return HashByte_Global(number, max, noise, offset);
};

s16 HashPercent(int number, int noise, int offset, int global, int earlygamePromo){
	if (number < 0) number = 0;
	int variation = (RandValues.variance)*5;
	int percentage = 0; 
	if (global) { 
		percentage = HashByte_Global(number, variation*2, noise, offset); //rn up to 150 e.g. 125
	} 
	else { percentage = HashByte_Ch(number, variation*2, noise, offset); }  //rn up to 150 e.g. 125
	percentage += (100-variation); // 125 + 25 = 150
	if (earlygamePromo == 1) { if (percentage > 125) { percentage = percentage / 2; } }
	if (earlygamePromo == 2) { if (percentage > 150) { percentage = percentage / 2; } }
	int ret = (percentage * number) / 100; //1.5 * 120 (we want to negate this)
	if (ret > 127) ret = (200 - percentage) * number / 100;
	if (ret < 0) ret = 0;
	return ret;
};

s16 HashByPercent_Ch(int number, int noise, int offset, int earlygamePromo){ 
	return HashPercent(number, noise, offset, false, earlygamePromo);
};

s16 HashByPercent(int number, int noise, int offset){
	return HashPercent(number, noise, offset, true, false);
};

// 80442AC enable co power if mov r0, #1, bx lr 
const s8 PowModifiers[] = { -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, -30, -20 } ; 
const s8 CoPowModifiers[] = { -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 80, 80, -10, 0 } ; 
const s8 SCoPowModifiers[] = { 10, 20, 30, 40, 50, 60, 70, 80, 80, 80, 80, 80, 10, 20 } ; 



const s8 DefModifiers[] = { -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, -30, -20, 0, 20, 30 } ; 
const s8 CoDefModifiers[] = { -20, -10, 0, 10, 20, 30, 40, 50, 60, 60, -20, -10, 10, 30, 40 } ; 
const s8 SCoDefModifiers[] = { -10, 0, 10, 20, 30, 40, 50, 60, 60, 60, -10, 0, 20, 40, 50 } ; 

enum { 
none1, inf, mech, mdt, apc, tank, recon, tcop, neo, lndr, artl, rckt, none2, none3, anti, miss,
fighter, bomber, none4, copter, none5, bship, cruiser, none6, sub}; 

//const int PowModifiers[13] = { -30, -20, -10, 0, 10, 15, 20, 30, 40, 50, 60, 70, 75 } ; 
int HashPow(int number, int noise, int offset, int coPow) {  
	LoadDesignRoom1Name(); 
	const s8* table = PowModifiers; 
	int size = sizeof(PowModifiers); 
	switch (coPow) { 
		case 0x44: { 
		table = CoPowModifiers; 
		size = sizeof(CoPowModifiers); 
		offset += 41; // make it reroll stats using the better modifiers list 
		break; 
		} 
		case 0x88: { 
		table = SCoPowModifiers; 
		size = sizeof(SCoPowModifiers); 
		offset += 66; // make it reroll stats using the better modifiers list 
		break; 
		} 
		default: 
	} 
	
	
	int result = HashByte_Global(number, size, noise, offset);
	return table[result];   
};  
int HashDef(int number, int noise, int offset, int coPow) { 
	LoadDesignRoom1Name(); 
	const s8* table = DefModifiers; 
	int size = sizeof(DefModifiers); 
	switch (coPow) { 
		case 0x44: { 
		table = CoDefModifiers; 
		size = sizeof(CoDefModifiers); 
		offset += 41; // make it reroll stats using the better modifiers list 
		break;
		} 
		case 0x88: { 
		table = SCoDefModifiers; 
		size = sizeof(SCoDefModifiers); 
		offset += 66; // make it reroll stats using the better modifiers list 
		break; 
		} 
		default: 
	} 
	
	
	int result = HashByte_Global(number, size, noise, offset);
	result = table[result];
	// lash max 20 def in sco 
	if ((noise == 0xC) && (coPow == 0x88)) { if (result > 20) { result = 20; } } 
	return result;   
};  

const s8 MovModifiers[] = { -1, -2, 0, 0, 0, 1, 2, 3 } ;  
const s8 CoMov[] = { 0, -1, 1, 0, 0, 0, 0, 2, 2, 1 } ;  
const s8 SCoMov[] = { 1, 0, 0, 0, 0, 1, 2, 3, 3, 3 } ;  
const s8 InfantryMov[] = { 0, 0, 0, 0, 0, 1, 2, 3, -1} ; 
const s8 CoInfantryMov[] = { 0, 0, 0, 2, 1, 1, 1, 1, 2, 3, -1} ; 
const s8 SCoInfantryMov[] = { 0, 0, 0, 4, 1, 1, 2, 6, 2, 3, 5} ; 
const s8 MechMov[] = { 0, 0, 0, 0, 0, 1, 2, 1, 3 } ; 
const s8 CoMechMov[] = { 0, 0, 0, 0, 1, 2, 4, 1, 2, 1, 3 } ; 
const s8 SCoMechMov[] = { 0, 0, 0, 0, 3, 4, 5, 1, 2, 1, 3 } ; 
const s8 ReconMov[] = { 0, 0, 0, 0, 0, 1, -1, -2, -3, -4} ; 
const s8 CoReconMov[] = { 0, 0, 0, 0, 0, 1, 1, 0, -1, -2} ; 
const s8 SCoReconMov[] = { 0, 0, 0, 1, 1, 1, 1, 1, -1, -2} ; 
const s8 FighterMov[] = { 0, 0, 0, 0, 0, -1, -2, -3, -4, -1, -2 } ; 
const s8 CoFighterMov[] = { 0, 0, 0, 0, 0, 0, -1, -2, -3, 0, -1 } ; 
const s8 SCoFighterMov[] = { 0, 0, 0, 0, 0, 0, 0, -1, -2, 0, 0 } ; 
const s8 BomberMov[] = { 0, 0, 0, 0, 0, 1, 2, -1, -2, -3, -4, -1, -2 } ; 
const s8 CoBomberMov[] = { 0, 0, 0, 0, 1, 1, 2, 1, 2, -1, -2, 0, 0 } ; 
const s8 SCoBomberMov[] = { 0, 1, 1, 1, 2, 1, 2, 1, 2, -1, -2, 0, 0 } ; 
int HashMov(int number, int noise, int offset, int coPow) { 
	//int result = HashByte_Global(number, (max_mov - min_mov)+1, noise, noise);
	//result = (max_mov - result) + min_mov; 
	LoadDesignRoom1Name(); 
	const s8* table = MovModifiers; 
	int size = sizeof(MovModifiers);
	 
	switch (coPow) { 
		case 0x44: { 
			switch (offset) { 
				case inf: {
					table = CoInfantryMov; 
					size = sizeof(CoInfantryMov); offset += 41;
					break; 
				}
				case mech: {
					table = CoMechMov; 
					size = sizeof(CoMechMov); offset += 41;
					break; 
				}
				case recon: {
					table = CoReconMov; 
					size = sizeof(CoReconMov); offset += 41;
					break; 
				}
				case fighter: {
					table = CoFighterMov; 
					size = sizeof(CoFighterMov); offset += 41;
					break; 
				}
				case bomber: {
					table = CoBomberMov; 
					size = sizeof(CoBomberMov); offset += 41;
					break; 
				}
				default: {
					table = CoMov; 
					size = sizeof(CoMov); offset += 41;
				} 
			}
		break;
		} 
		case 0x88: { 
			switch (offset) { 
				case inf: {
					table = SCoInfantryMov; 
					size = sizeof(SCoInfantryMov); offset += 66;
					break; 
				}
				case mech: {
					table = SCoMechMov; 
					size = sizeof(SCoMechMov); offset += 66;
					break; 
				}
				case recon: {
					table = SCoReconMov; 
					size = sizeof(SCoReconMov); offset += 66;
					break; 
				}
				case fighter: {
					table = SCoFighterMov; 
					size = sizeof(SCoFighterMov); offset += 66;
					break; 
				}
				case bomber: {
					table = SCoBomberMov; 
					size = sizeof(SCoBomberMov); offset += 66;
					break; 
				}
				default: {
					table = SCoMov; 
					size = sizeof(SCoMov); offset += 66;
				} 
			}
		break; 
		} 
		default: {
			switch (offset) { 
				case inf: {
					table = InfantryMov; 
					size = sizeof(InfantryMov); 
					break; 
				}
				case mech: {
					table = MechMov; 
					size = sizeof(MechMov); 
					break; 
				}
				case recon: {
					table = ReconMov; 
					size = sizeof(ReconMov); 
					break; 
				}
				case fighter: {
					table = FighterMov; 
					size = sizeof(FighterMov); 
					break; 
				}
				case bomber: {
					table = BomberMov; 
					size = sizeof(BomberMov); 
					break; 
				}
				default: 
			}
		}
	} 


	

	
	
	int result = HashByte_Global(number, size, noise, offset + 21);
	return table[result];   
}; 

const s8 RangeModifiers[] = { 0, 0, 0, 0, 0, 1, 2, 1, 2 } ; 
const s8 CoRange[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4 } ; 
const s8 SCoRange[] = { 1, 1, 5, 1, 1, 1, 2, 3, 4 } ; 
const s8 ArtilleryRange[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4, -1} ; 
const s8 CoArtilleryRange[] = { 1, 1, 1, 2, 3, 1, 2, 3, 4, -1} ; 
const s8 SCoArtilleryRange[] = { 1, 1, 2, 3, 4, 1, 2, 3, 4, 1} ; 
const s8 RocketRange[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4, -1, -2 } ; 
const s8 CoRocketRange[] = { 0, 1, 2, 1, 2, 1, 2, 3, 4, -1, -1 } ; 
const s8 SCoRocketRange[] = { 1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1 } ; 
const s8 MissileRange[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4, -1, -2 } ; 
const s8 CoMissileRange[] = { 0, 1, 2, 1, 2, 1, 2, 3, 4, -1, 0 } ; 
const s8 SCoMissileRange[] = { 1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1 } ; 
const s8 BattleshipRange[] = { 0, 0, 0, 0, 0, 1, 2, -1, -2 } ; 
const s8 CoBattleshipRange[] = { 0, 1, 1, 2, 3, 1, 2, -1, 0 } ; 
const s8 SCoBattleshipRange[] = { 1, 1, 2, 3, 2, 1, 2, 1, 1 } ; 
int HashRange(int number, int noise, int offset, int otherNum, int coPow) { 
	//int result = HashByte_Global(number, (max_range - min_range)+1, noise, noise);
	//result = (max_range - result) + min_range; 
	LoadDesignRoom1Name(); 
	
	const s8* table = RangeModifiers; 
	int size = sizeof(RangeModifiers); 
	
	if (HashMov(otherNum, noise, offset, coPow)) { return 0; } 

	switch (coPow) { 
		case 0x44: {  
			switch (offset) { 
				case artl: {
					table = CoArtilleryRange; 
					size = sizeof(CoArtilleryRange); offset += 41;
					break; 
				} 
				case rckt: {
					table = CoRocketRange; 
					size = sizeof(CoRocketRange); offset += 41;
					break; 
				} 
				case miss: {
					table = CoMissileRange; 
					size = sizeof(CoMissileRange); offset += 41;
					break; 
				} 
				case bship: {
					table = CoBattleshipRange; 
					size = sizeof(CoBattleshipRange); offset += 41;
					break; 
				} 
				default: {
					table = CoRange; 
					size = sizeof(CoRange); 
				} 
			}
		break;
		} 
		case 0x88: {  
			switch (offset) { 
				case artl: {
					table = SCoArtilleryRange; 
					size = sizeof(SCoArtilleryRange); offset += 66;
					break; 
				} 
				case rckt: {
					table = SCoRocketRange; 
					size = sizeof(SCoRocketRange); offset += 66;
					break; 
				} 
				case miss: {
					table = SCoMissileRange; 
					size = sizeof(SCoMissileRange); offset += 66;
					break; 
				} 
				case bship: {
					table = SCoBattleshipRange; 
					size = sizeof(SCoBattleshipRange); offset += 66;
					break; 
				} 
				default: {
					table = SCoRange; 
					size = sizeof(SCoRange); offset += 66;
				} 
			}
		break;
		} 
		default: {  
			switch (offset) { 
				case artl: {
					table = ArtilleryRange; 
					size = sizeof(ArtilleryRange); 
					break; 
				} 
				case rckt: {
					table = RocketRange; 
					size = sizeof(RocketRange); 
					break; 
				} 
				case miss: {
					table = MissileRange; 
					size = sizeof(MissileRange); 
					break; 
				} 
				case bship: {
					table = BattleshipRange; 
					size = sizeof(BattleshipRange); 
					break; 
				} 
				default: 
			}
		break;
		} 
	}

	
	int result = HashByte_Global(number, size, noise, offset + 31);
	return table[result];   
}; 
