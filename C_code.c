
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
	u32 seed : 20; // max value of 999999 /  
	u32 variance : 5; // up to 5*31 / 100% 
	u32 bonus : 5; // up to +31 / +20 levels 
}; 

extern struct RandomizerSettings RandBitflags; 
extern struct RandomizerValues RandValues; 
extern u8 gCh; 
extern u32 gGameClock;


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

u16 HashByte_Global(int number, int max, u8 noise[], int offset) {
	if (max==0) return 0;
	offset = Mod(offset, 256); 
	u32 hash = 5381;
	hash = ((hash << 5) + hash) ^ number;
	//hash = ((hash << 5) + hash) ^ *StartTimeSeedRamLabel;
	u8 seed[3] = { (RandValues.seed & 0xFF), (RandValues.seed&0xFF00)>>8, (RandValues.seed&0xFF0000)>>16 }; 
	for (int i = 0; i < 3; ++i){
		hash = ((hash << 5) + hash) ^ seed[i];
	};
	int noisy = noise[0] | (noise[1] << 8) | (noise[2] << 16) | (noise[3] << 24); 
	
	//u16 currentRN[3] = { 0, 0, 0 }; 
	hash = GetNthRN(offset + 1, noisy+hash); 
	//InitSeededRN(hash + seed, currentRN);
	//hash = NextSeededRN(currentRN); 
	//for (i = 0; i < 9; i++) { 
		//if (!noise[i]) { break; } 
		//hash = ((hash << 5) + hash) ^ noise[i];
	//} 
	
	return Mod((hash & 0x2FFFFFFF), max);
};

u16 HashByte_Ch(int number, int max, u8 noise[], int offset){
	int i = 0; 
	for (i = 0; i < 9; i++) { 
		if (!noise[i]) { break; } 
	} 
	noise[i+1] = gCh; 
	noise[i+2] = 0; 
	return HashByte_Global(number, max, noise, offset);
};

s16 HashPercent(int number, u8 noise[], int offset, int global, int earlygamePromo){
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

s16 HashByPercent_Ch(int number, u8 noise[], int offset, int earlygamePromo){ 
	return HashPercent(number, noise, offset, false, earlygamePromo);
};

s16 HashByPercent(int number, u8 noise[], int offset){
	return HashPercent(number, noise, offset, true, false);
};

