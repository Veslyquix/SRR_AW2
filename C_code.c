
#include "include/aw2.h"
#define ABS(aValue) ((aValue) >= 0 ? (aValue) : -(aValue))
struct RandomizerSettings {
  u16 base : 1;
  u16 growth : 2; // vanilla, randomized, 0%, 100%
  u16 caps : 2;   // vanilla, randomized, 30
  u16 class : 1;
  u16 itemStats : 1;
  u16 foundItems : 1;
  u16 shopItems : 1;
  u16 disp : 1;
};

struct RandomizerValues {
  u32 seed : 31;
  u32 variance : 1;
  // u32 bonus : 5;
};

// 8034dcc update music
extern struct RandomizerSettings RandBitflags;
extern struct RandomizerValues RandValues;
extern u8 gCh;
extern u32 gGameClock;

// 0x0803CC84 - Loads Design Map 1/2/3 Names into RAM
extern void LoadDesignRoomName(int, int); // 0x803CC84
void LoadDesignRoom1Name(void) { LoadDesignRoomName(0x200B204, 0x20280C2); }

#define SHORTCALL __attribute__((short_call))
#define CONSTFUNC __attribute__((const))
extern int ModNum(int a, int b) SHORTCALL;
int NextSeededRN(u16 *currentRN) {
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

void InitSeededRN(int seed, u16 *currentRN) {
  // This table is a collection of 8 possible initial rn state
  // 3 entries will be picked based of which "seed" was given

  u16 initTable[8] = {0xA36E, 0x924E, 0xB784, 0x4F67,
                      0x8092, 0x592D, 0x8E70, 0xA794};

  int mod = ModNum(seed, 7);

  currentRN[0] = initTable[(mod++ & 7)];
  currentRN[1] = initTable[(mod++ & 7)];
  currentRN[2] = initTable[(mod & 7)];

  if (ModNum(seed, 13) > 0)
    for (mod = ModNum(seed, 13); mod != 0; mod--)
      NextSeededRN(currentRN);
}

u16 GetNthRN(int n, int seed) {
  u16 currentRN[3] = {0, 0, 0};
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
    result =
        0; // TacticianName[1] | (TacticianName[2]<<8) | (TacticianName[3]<<16);
    int clock = gGameClock;
    result = (GetNthRN(clock, result) << 4) | GetNthRN(clock, result);
  }
  if (result > 999999) {
    result &= 0xEFFFF;
  }
  return result;
}

extern u8 DesignRoom1Name[12];
u16 HashByte_Global(int number, int max, int noise, int offset) {
  if (max == 0)
    return 0;
  offset = ModNum(offset, 32);
  u32 hash = 5381;
  hash = ((hash << 5) + hash) ^ number;
  // hash = ((hash << 5) + hash) ^ *StartTimeSeedRamLabel;
  for (int i = 0; i < 12; ++i) {
    if (!DesignRoom1Name[i]) {
      break;
    }
    hash = ((hash << 5) + hash) ^ DesignRoom1Name[i];
  };

  u8 seed[3] = {(noise & 0xFF), (noise & 0xFF00) >> 8,
                (noise & 0xFF0000) >> 16};
  for (int i = 0; i < 3; ++i) {
    if (!seed[i]) {
      break;
    }
    hash = ((hash << 5) + hash) ^ seed[i];
  };
  hash = GetNthRN(offset + 1, hash);
  return ModNum((hash & 0x2FFFFFFF), max);
};

u16 HashByte_Ch(int number, int max, int noise, int offset) {
  noise += (gCh << 8);
  return HashByte_Global(number, max, noise, offset);
};

s16 HashByte_ChIfConfig(int number, int max, int noise, int offset) {
  if (DesignRoom1Name[0] == 0x41 ||
      DesignRoom1Name[0] == 0x43) { // "A" or "C" = no random stats
    return (-1);
  }
  if (DesignRoom1Name[0] == 0x20) { // blank char
    // asm("mov r11, r11");
    noise += (gCh << 8);
    if (DesignRoom1Name[1] == 0x41 || DesignRoom1Name[1] == 0x43) {
      return (-1);
    }
  }
  return HashByte_Global(number, max, noise, offset);
};

s16 HashPercent(int number, int noise, int offset, int global,
                int earlygamePromo) {
  if (number < 0)
    number = 0;
  int variation = (RandValues.variance) * 5;
  int percentage = 0;
  if (global) {
    percentage = HashByte_Global(number, variation * 2, noise,
                                 offset); // rn up to 150 e.g. 125
  } else {
    percentage = HashByte_Ch(number, variation * 2, noise, offset);
  } // rn up to 150 e.g. 125
  percentage += (100 - variation); // 125 + 25 = 150
  if (earlygamePromo == 1) {
    if (percentage > 125) {
      percentage = percentage / 2;
    }
  }
  if (earlygamePromo == 2) {
    if (percentage > 150) {
      percentage = percentage / 2;
    }
  }
  int ret = (percentage * number) / 100; // 1.5 * 120 (we want to negate this)
  if (ret > 127)
    ret = (200 - percentage) * number / 100;
  if (ret < 0)
    ret = 0;
  return ret;
};

s16 HashByPercent_Ch(int number, int noise, int offset, int earlygamePromo) {
  return HashPercent(number, noise, offset, false, earlygamePromo);
};

s16 HashByPercent(int number, int noise, int offset) {
  return HashPercent(number, noise, offset, true, false);
};

// 80442AC enable co power if mov r0, #1, bx lr
const s8 PowModifiers[] = {-30, -20, -10, 0,  10, 20,  30,
                           40,  50,  60,  70, 80, -30, -20};
const s8 CoPowModifiers[] = {-10, 0,  10, 20, 30, 40,  50,
                             60,  70, 80, 80, 80, -10, 0};
const s8 SCoPowModifiers[] = {10, 20, 30, 40, 50, 60, 70,
                              80, 80, 80, 80, 80, 10, 20};

const s8 DefModifiers[] = {-30, -20, -10, 0,   10, 20, 30, 40,
                           50,  10,  -30, -20, 0,  20, 30};
const s8 CoDefModifiers[] = {-20, -10, 0,   10,  20, 30, 40, 50,
                             60,  60,  -20, -10, 10, 30, 40};
const s8 SCoDefModifiers[] = {-10, 0,  10,  20, 30, 40, 50, 60,
                              60,  60, -10, 0,  20, 40, 50};

enum {
  none1,
  inf,
  mech,
  mdt,
  apc,
  tank,
  recon,
  tcop,
  neo,
  lndr,
  artl,
  rckt,
  none2,
  none3,
  anti,
  miss,
  fighter,
  bomber,
  none4,
  copter,
  none5,
  bship,
  cruiser,
  none6,
  sub
};

struct PlayerStruct {
  u32 funds;     // P1 Funds
  u32 spent;     // P1 Total Funds spent
  u32 income;    // P1 Current Income
  u8 bases;      // P1's Owned Bases Total
  u8 cities;     // P1's Owned Cities Total
  u8 airports;   // P1's Owned Airports Total
  u8 ports;      // P1's Owned Ports Total
  u8 properties; // P1 Properties Owned
  u8 captures;   // P1's Properties Captured?
  // padding
  u16 padding1;
  u8 defeated;           // =01 when P1 is defeated
  u16 destroyedThisTurn; // Stores count of units destroyed this turn
  u16 totalDestroyed;    // Stores count of maximum units destroyed
  u8 teamColour;         // Player 1 Team Colour
  u8 aiControlled;       // 00 = None, 01 = Human, 02 = AI
  u8 turnState;          // =03 when P1's Turn
  u8 co;                 // P1 CO

  /*
  020232DE		: P1 CO Mode (00 = Normal, 01 = COP, 02 = SCOP)
  020232DF		: P1 CO Mode (Used for COP Activation)
  020232E0 W		: P1 S/COP Charge
  020232E4		: P1, which COP noise to play/Is COP/SCOP ready?
  020232E5		: P1 CO Power Uses
  020232E6 		: Temp Firepower Bonus
  020232E8 		: Temp Defence Bonus
  020232EA		: P1's Team (Used for attacking)
  020232EC		: Stores a team related byte - Appears to be the used
  one, others used for setup 020232ED		: P1's HQ X Co-ordinate 020232EE
  : P1's HQ Y Co-ordinate 020232EF		: P1's Selection Cursor X
  Co-ordinate 020232F0		: P1's Selection Cursor Y Co-ordinate 020232F2
  : If set to True, Kill Player on next End Turn. 020232F3		: Rank
  Earned 020232F4		: Points for Speed Score 020232F5
  : Points for Power Score 020232F6		: Points for Technique Score
  020232F8		: Total Score
  020232FA		: Stores Unit count
  020232FB		: Stores Unit Lost Count
  */
};

extern struct PlayerStruct gPlayer0; // neutral properties 02023284
extern struct PlayerStruct gPlayer1; // 20232C0
extern struct PlayerStruct gPlayer2; // 20232FC
extern struct PlayerStruct gPlayer3; // 2023338
extern struct PlayerStruct gPlayer4; // 2023374

int IsCoAiControlled(int co) {
  int result = false;
  if (gPlayer1.co == co) {
    if (gPlayer1.aiControlled == 2) {
      result = true;
    }
  }
  if (gPlayer2.co == co) {
    if (gPlayer2.aiControlled == 2) {
      result = true;
    }
  }
  if (gPlayer3.co == co) {
    if (gPlayer3.aiControlled == 2) {
      result = true;
    }
  }
  if (gPlayer4.co == co) {
    if (gPlayer4.aiControlled == 2) {
      result = true;
    }
  }
  // asm("mov r11, r11");

  return result;
}

// const int PowModifiers[13] = { -30, -20, -10, 0, 10, 15, 20, 30, 40, 50, 60,
// 70, 75 } ;
int HashPow(int number, int noise, int offset, int coPow) {
  LoadDesignRoom1Name();
  const s8 *table = PowModifiers;
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

  int result = HashByte_ChIfConfig(number, size, noise, offset);
  if (result < 0) {
    return number;
  }

  return table[result];
};
int HashDef(int number, int noise, int offset, int coPow) {
  LoadDesignRoom1Name();
  const s8 *table = DefModifiers;
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

  int result = HashByte_ChIfConfig(number, size, noise, offset);
  if (result < 0) {
    return number;
  }
  result = table[result];
  // lash max 20 def in sco
  if ((noise == 0xC) && (coPow == 0x88)) {
    if (result > 10) {
      result = 10;
    }
  }
  if (IsCoAiControlled(noise)) {
    if (result > 40) {
      result = 40;
    }
  }
  return result;
};

const s8 MovModifiers[] = {-1, -2, 0, 0, 0, 1, 2, 3};
const s8 CoMov[] = {0, -1, 1, 0, 0, 0, 0, 2, 2, 1};
const s8 SCoMov[] = {1, 0, 0, 0, 0, 1, 2, 3, 4, 3};
const s8 RangedMovModifiers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 1, 2, 3};
const s8 RangedCoMov[] = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0}; // so they can get a +range boost instead
const s8 RangedSCoMov[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const s8 InfantryMov[] = {0, 0, 0, 0,  0, 1, 2, 1, -1, 0, 0, 0, 0,  0,
                          1, 2, 3, -1, 0, 0, 0, 0, 0,  1, 2, 3, -1, 8};
const s8 CoInfantryMov[] = {0, 0, 0, 2, 1, 1, 1, 1, 2, 3, -1, 8};
const s8 SCoInfantryMov[] = {0, 0, 0, 4, 1, 1, 2, 6, 2, 3, 5, 8};
const s8 MechMov[] = {0, 0, 0, 0, -1, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1,
                      1, 0, 0, 0, 0,  0, 0, -1, 1, 1, 0, 0, 2, 3};
const s8 CoMechMov[] = {0, 0, 0, 0, 1, 2, 4, 1, 2, 1, 3};
const s8 SCoMechMov[] = {0, 0, 0, 0, 3, 4, 5, 1, 2, 1, 3};
const s8 ReconMov[] = {0, 0, 1, 2, 3, -1, -2, 1, 2, 3};
const s8 CoReconMov[] = {0, 0, 1, 2, 3, 1, 1, 1, 2, 3};
const s8 SCoReconMov[] = {0, 0, 2, 2, 3, 1, 1, 1, 2, 3};
const s8 FighterMov[] = {0, 0, 0, 0, 0, -1, -2, -3, -4, 1, 2};
const s8 CoFighterMov[] = {0, 0, 0, 0, 0, 1, -1, -2, -3, 1, 2};
const s8 SCoFighterMov[] = {0, 0, 0, 0, 0, 1, 2, -1, -2, 2, 1};
const s8 BomberMov[] = {0, 0, 0, 0, 3, 1, 2, -1, -2, -3, -4, -1, -2};
const s8 CoBomberMov[] = {0, 0, 0, 0, 1, 1, 3, 1, 2, -1, -2, 0, 0};
const s8 SCoBomberMov[] = {0, 1, 1, 1, 2, 1, 3, 1, 3, -1, 2, 0, 0};
int HashMov(int number, int noise, int offset, int coPow) {
  // int result = HashByte_Global(number, (max_mov - min_mov)+1, noise, noise);
  // result = (max_mov - result) + min_mov;
  LoadDesignRoom1Name();
  const s8 *table = MovModifiers;
  int size = sizeof(MovModifiers);

  switch (coPow) {
  case 0x44: {
    switch (offset) {
    case inf: {
      table = CoInfantryMov;
      size = sizeof(CoInfantryMov);
      offset += 41;
      break;
    }
    case mech: {
      table = CoMechMov;
      size = sizeof(CoMechMov);
      offset += 41;
      break;
    }
    case recon: {
      table = CoReconMov;
      size = sizeof(CoReconMov);
      offset += 41;
      break;
    }
    case fighter: {
      table = CoFighterMov;
      size = sizeof(CoFighterMov);
      offset += 41;
      break;
    }
    case bomber: {
      table = CoBomberMov;
      size = sizeof(CoBomberMov);
      offset += 41;
      break;
    }
    case artl:
    case rckt:
    case miss:
    case bship: {
      table = RangedCoMov;
      size = sizeof(RangedCoMov);
      offset += 41;
      break;
    }
    default: {
      table = CoMov;
      size = sizeof(CoMov);
      offset += 41;
    }
    }
    break;
  }
  case 0x88: {
    switch (offset) {
    case inf: {
      table = SCoInfantryMov;
      size = sizeof(SCoInfantryMov);
      offset += 66;
      break;
    }
    case mech: {
      table = SCoMechMov;
      size = sizeof(SCoMechMov);
      offset += 66;
      break;
    }
    case recon: {
      table = SCoReconMov;
      size = sizeof(SCoReconMov);
      offset += 66;
      break;
    }
    case fighter: {
      table = SCoFighterMov;
      size = sizeof(SCoFighterMov);
      offset += 66;
      break;
    }
    case bomber: {
      table = SCoBomberMov;
      size = sizeof(SCoBomberMov);
      offset += 66;
      break;
    }
    case artl:
    case rckt:
    case miss:
    case bship: {
      table = RangedSCoMov;
      size = sizeof(RangedSCoMov);
      offset += 66;
      break;
    }
    default: {
      table = SCoMov;
      size = sizeof(SCoMov);
      offset += 66;
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
    case artl:
    case rckt:
    case miss:
    case bship: {
      table = RangedMovModifiers;
      size = sizeof(RangedMovModifiers);
      break;
    }
    default:
    }
  }
  }

  int result = HashByte_ChIfConfig(number, size, noise, offset + 21);
  if (result < 0) {
    return number;
  }
  return table[result];
};

const s8 RangeModifiers[] = {0, 0, 0, 0, 0, 1, 2, 1, 2};
const s8 CoRange[] = {0, 0, 0, 0, 0, 1, 2, 3, 4};
const s8 SCoRange[] = {1, 1, 5, 1, 1, 1, 2, 3, 4};
const s8 AiDirectCmbRange[] = {0, 0, 0, 0};
const s8 ArtilleryRange[] = {0, 0, 0, 1, 1, 1, 2, 3, 4, -1};
const s8 CoArtilleryRange[] = {1, 1, 1, 2, 3, 1, 2, 3, 4, -1};
const s8 SCoArtilleryRange[] = {1, 1, 2, 3, 4, 1, 2, 3, 4, 1};
const s8 RocketRange[] = {0, 0, 0, 1, 1, 1, 2, 3, 4, -1, -2};
const s8 CoRocketRange[] = {0, 1, 2, 1, 2, 1, 2, 3, 4, -1, -1};
const s8 SCoRocketRange[] = {1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1};
const s8 MissileRange[] = {0, 0, 0, 1, 1, 1, 2, 3, 4, -1, -2};
const s8 CoMissileRange[] = {0, 1, 2, 1, 2, 1, 2, 3, 4, -1, 0};
const s8 SCoMissileRange[] = {1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 1};
const s8 BattleshipRange[] = {0, 0, 0, 1, 1, 1, 2, -1, -2};
const s8 CoBattleshipRange[] = {0, 1, 1, 2, 3, 1, 2, -1, 0};
const s8 SCoBattleshipRange[] = {1, 1, 2, 3, 2, 1, 2, 1, 1};
int HashRange(int number, int noise, int offset, int otherNum, int coPow) {
  // int result = HashByte_Global(number, (max_range - min_range)+1, noise,
  // noise); result = (max_range - result) + min_range;
  LoadDesignRoom1Name();

  const s8 *table = RangeModifiers;
  int size = sizeof(RangeModifiers);
  int aiControlled = IsCoAiControlled(noise);
  if (aiControlled) {
    table = AiDirectCmbRange;
    size = sizeof(AiDirectCmbRange);
  }
  if ((offset == inf) || (offset == recon)) {
    return 0;
  }

  if (HashMov(otherNum, noise, offset, coPow) > 0) {
    return 0;
  }

  switch (coPow) {
  case 0x44: {
    switch (offset) {
    case artl: {
      table = CoArtilleryRange;
      size = sizeof(CoArtilleryRange);
      offset += 41;
      break;
    }
    case rckt: {
      table = CoRocketRange;
      size = sizeof(CoRocketRange);
      offset += 41;
      break;
    }
    case miss: {
      table = CoMissileRange;
      size = sizeof(CoMissileRange);
      offset += 41;
      break;
    }
    case bship: {
      table = CoBattleshipRange;
      size = sizeof(CoBattleshipRange);
      offset += 41;
      break;
    }
    default: {
      table = CoRange;
      size = sizeof(CoRange);
      if (aiControlled) {
        table = AiDirectCmbRange;
        size = sizeof(AiDirectCmbRange);
      }
    }
    }
    break;
  }
  case 0x88: {
    switch (offset) {
    case artl: {
      table = SCoArtilleryRange;
      size = sizeof(SCoArtilleryRange);
      offset += 66;
      break;
    }
    case rckt: {
      table = SCoRocketRange;
      size = sizeof(SCoRocketRange);
      offset += 66;
      break;
    }
    case miss: {
      table = SCoMissileRange;
      size = sizeof(SCoMissileRange);
      offset += 66;
      break;
    }
    case bship: {
      table = SCoBattleshipRange;
      size = sizeof(SCoBattleshipRange);
      offset += 66;
      break;
    }
    default: {
      table = SCoRange;
      size = sizeof(SCoRange);
      offset += 66;
      if (aiControlled) {
        table = AiDirectCmbRange;
        size = sizeof(AiDirectCmbRange);
      }
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

  int result = HashByte_ChIfConfig(number, size, noise, offset + 31);
  if (result < 0) {
    return number;
  }
  return table[result];
};

//
// function address 2163c InitMap ?
// bl 247A4 sets 3003f68
// bl 8014e44 -> 8014dcc decides on address to store into 3003f68 based on
// 3000050 map size at 21666 load 3003f68 // [3003f68]!! pointer to current
// buffer ? store to 201e450
// [201e450..201e451]?
// 215fc

// around 216fe it sets the terrain for the map
// [201f8c4]!! // terrain
// [201eeb0]!! // tiles

// map size pointer: 8499590
// gMapTile[y][x]: 849959c ?

// r9 as 3003f68 poin 2003010
//

// [2003012]!! // tiles for map

//
// size + 0x417a offset: copy of map size / header stuff?

// 20225f0

// [201ee72]!!
// f4e0 - make road at coords?
// to 201E450 ram / 201EE72

extern void MakeTile(void); // uses above three ram
extern u16 mapTileData[];
extern int NumberOfMapPieces;
extern int FrequencyOfObjects_Link;
struct Map_Struct {
  u8 x;
  u8 y;
  u16 data[];
};
struct ChHeader {
  u16 x;
  u16 y;
};

extern struct Map_Struct *MapPiecesTable[0xFF];

void GenerateMap(struct Map_Struct *dst, struct ChHeader *head, int chID);
void CopyMapPiece(u16 dst[], u8 xx, u8 yy, u8 map_size_x, u8 map_size_y, int);
extern int RandomizeMaps;
int ShouldMapBeRandomized(void) {
  LoadDesignRoom1Name();
  // DesignRoom1Name[0] = 2; // don't randomize if design room 1 name starts
  // with "B" or "C"
  if (DesignRoom1Name[0] == 0x42 || DesignRoom1Name[0] == 0x43) {
    return false;
  }
  if (DesignRoom1Name[0] == 0x20) {
    if (DesignRoom1Name[1] == 0x42 || DesignRoom1Name[1] == 0x43) {
      return false;
    }
  }
  return RandomizeMaps; // change to return false; later
}
struct Vec2u {
  int x;
  int y;
};

extern int Rand(int val);
int NextRN(int val) {
  if (!val) {
    val = 0x29ae6736;
  }
  val = Rand(val);
  return val;
}

extern void MakeRoad(int x, int y);
extern void MakeRoadSimple(int x, int y);
extern void MakeTileSimple(int x, int y, int tile);
extern void MakeMountain(int x, int y, int one);
extern void MakeForest(int x, int y, int one);
extern void MakeForestSimple(int x, int y, int one);
extern void MakeRiver(int x, int y);
extern void MakeSea(int x, int y, int one);
extern void MakeSeaSafe(int x, int y, int one);
extern void MakeSeaSafest(int x, int y);
extern void MakeBridge(int x, int y, int one);
extern void MakeShoal(int x, int y, int one);
extern void MakePipe(int x, int y);
extern void MakeSeam(int x, int y, int one);
extern void MakeReefSafe(int x, int y, int one);

#define HQ_OS (0x714 >> 2)
#define BASE_OS (0x718 >> 2)
#define HQ_BM (0x728 >> 2)
#define BASE_BM (0x72c >> 2)
#define HQ_GE (0x73C >> 2)
#define BASE_GE (0x740 >> 2)
#define HQ_YS (0x750 >> 2)
#define BASE_YS (0x754 >> 2)

#define Mountain 0x80 >> 2
#define Plain 0x4 >> 2
#define Forest 0x21C >> 2
#define Sea 0x20 >> 2  // 1 tile of sea in plains
#define SeaC 0xA8 >> 2 // connecting to sea
#define Reef 0x5A0 >> 2

#define HQ 0x700 >> 2
#define Base 0x704 >> 2
#define City 0x708 >> 2
#define Airport 0x70c >> 2
#define Port 0x710 >> 2

#define Silo 0x600 >> 2

#define BridgeH (0x50 >> 2)
#define BridgeV (0x58 >> 2)
#define RoadV (0x100 >> 2)
#define RoadH (0x184 >> 2)

#define Laser 0x604 >> 2
#define Cannon 0x608 >> 2
#define Volcano 0x69C >> 2

// Design room definitions
// 0
#define _Plain 1
#define _River 2
#define _Mtn 3
#define _Wood 4
#define _Road 5
#define _City 6
#define _Sea 7
#define _HQ 8
// 9
#define _Arprt 0xA
#define _Port 0xB
#define _Brdg 0xC
#define _Shoal 0xD
#define _Base 0xE
#define _Pipe 0xF
#define _Seam 0x10
#define _Silo 0x11
// 0x12
#define _Reef 0x13

// #define ForceSizeX 20
// #define ForceSizeY 15
#define LineSegment
#define HQs
#define Pieces
#define Filler
// #define ForceTileBank defaultTiles
// #define ForceType _Sea

// Randomized wiggly line function without diagonal moves and ensuring no gaps
void drawWigglyRoad(int xA, int yA, int xB, int yB, int sizeX, int factionA,
                    int factionB, int id) {

  int x = xA, y = yA;
  // mapTileData[(y * sizeX) + x] = 0x104; // Mark the start point [201ee78]!!
  void (*func)(int x, int y) = MakeRoad; // default
  int tile = Plain;
  switch (id) {
  case _Road: {
    // tile = RoadH;
    tile = 0;
    func = MakeRoad;
    break;
  }
  case _Pipe: {
    func = MakePipe;
    break;
  }
  case _River: {
    func = MakeRiver;
    break;
  }
  case _Sea: {
    tile = SeaC;
    func = MakeSeaSafest;
    break;
  }
  default:
  }
  // void MyFunction(int x, int y) = MakeRoad();

  func(x, y); // Mark the start point
  int attempts = 0;
  int dir = 0;
  int backtrack = 0;
  int backtrackChance = 0;
  int freq = sizeX >> 2; // every n attempts, change direction
  if (freq < 3) {
    freq = 3;
  }
  if (freq > 6) {
    freq = 6;
  }

  while (x != xB || y != yB) {
    attempts++;
    if (attempts > 100000) {
      break;
    }
    backtrackChance = Mod(attempts, 8) < 4 ? 0 : 2;
    backtrackChance += attempts < 200 ? 2 : 5;
    if (!Mod(attempts, freq)) {
      dir = HashByte_Ch(
          dir, 2, x + id,
          attempts); // Randomly choose between 0 (horizontal) or 1 (vertical)
      backtrack = HashByte_Ch(backtrack, backtrackChance, y + id, attempts);
    }
    // int backtrack = ModNum(rand, 4); // 25% chance of backtracking
    if (tile == RoadH || tile == RoadV) {
      if (dir == 0) {
        tile = RoadH;
      } else {
        tile = RoadV;
      }
    }
    // Horizontal dir
    if (dir == 0) {
      if (backtrack == 0 && x != xA) {
        // Move horizontally away (backtrack)
        x += (xA > x) ? 1 : -1;
      } else if (x != xB) {
        // Move horizontally toward the goal
        x += (xB > x) ? 1 : -1;
      }
    }
    // Vertical move
    else if (dir == 1) {
      if (backtrack == 0 && y != yA) {
        // Move vertically away (backtrack)
        y += (yA > y) ? 1 : -1;
      } else if (y != yB) {
        // Move vertically toward the goal
        y += (yB > y) ? 1 : -1;
      }
    }
    //  Mark the new position in the array
    // mapTileData[(y * sizeX) + x] =
    // 0x104; // because MakeRoad doesn't write to mapTileData when only
    // displaying the preview, I guess?
    if (tile) {
      mapTileData[(y * sizeX) + x] = tile; // make it a plain first
    }
    func(x, y);

    // tiles adjacent to HQ will be a base
    // if ((ABS(x - xA) + ABS(y - yA)) == 1) {
    // mapTileData[(y * sizeX) + x] = BASE_OS + (factionA * 5);
    // } else if ((ABS(x - xB) + ABS(y - yB)) == 1) {
    // mapTileData[(y * sizeX) + x] = BASE_OS + (factionB * 5);
    // } else {
    // MakeRoad(x, y);
    // }
    //  dst->data[(y * map_size_x) + x] = mapTileData[(y * map_size_x) + x];
    //   dst->data[(y * map_size_x) + x] = value;
  }
  // mapTileData[(yA * sizeX) + xA] = HQ_OS + (factionA * 5);
  // mapTileData[(y * sizeX) + x] = HQ_OS + (factionB * 5);
  // mapTileData[(yB * sizeX) + xB] = HQ_BM;
}

void SetMapSize(struct Map_Struct *dst, struct ChHeader *head, int chID) {
  if (!ShouldMapBeRandomized()) {
    return;
  }
  int x = dst->x;
  int y = dst->y;
  if (x > 35) {
    x = 35;
  }
  if (y > 35) {
    y = 35;
  }
  x = HashByte_Ch(gCh, x, 5, 0) + x / 2; // 50% to 150% of usual
  y = HashByte_Ch(gCh, y, 5, 0) + y / 2;
  // set min / max
  if (x < 15) {
    x = 15;
  }
  if (y < 10) {
    y = 10;
  }
  if (x > 25) {
    x = 25;
  }
  if (y > 25) {
    y = 25;
  }
#ifdef ForceSizeX
  x = ForceSizeX;
#endif
#ifdef ForceSizeY
  y = ForceSizeY;
#endif
  dst->x = x;
  dst->y = y;
  head->x = dst->x;
  head->y = dst->y;
}

struct tileWeight {
  u16 tile;
  u16 weight;
};

extern u8 Unk_200B000[];
/*

extern u8 Unk_200B036;
extern u16 SelectedTile;
extern u16 SelectedTile_3A;
extern u16 PreviousTile_3C;
extern u16 SelectedTileX;
extern u16 SelectedTileY;
*/
/* 08 */ u32 save_magic32;
/* 0C */ u8 _pad_0C[0xF - 0xC];
/* 0F */ u8 unk0F;
// clang-format off
struct activeMap {
  u8 unk1[0x8-0]; 
  u16 SelectedTileX;
  u16 SelectedTileY;
  u8 unk2[0x12-0xC]; 
  u8 Surplus; // +0x12
  u8 unk3[0x2a-0x13]; 
  u16 SelectedTile; // press B in design room on tile 
  u16 unk2C; 
  u8 factionProperties; // Army for properties
  u8 factionUnits; // Which army the menu units belong to
  u8 unk4[0x36-0x30]; 
  u16 unk36; 
  u16 unk38; 
  u16 unk3a;
  u16 previousTile; 
};
// clang-format on
extern struct activeMap *gActiveMap;

const struct tileWeight defaultTiles[] = {
    {_Plain, 85}, {_River, 0}, {_Mtn, 45},  {_Wood, 45}, {_Road, 40},
    {_City, 25},  {_Sea, 55},  {_Arprt, 4}, {_Port, 1},  {_Brdg, 15},
    {_Shoal, 15}, {_Base, 15}, {_Pipe, 0},  {_Silo, 12}, {_Reef, 0},
};

const struct tileWeight mountainousTiles[] = {
    {_Plain, 15}, {_River, 0}, {_Mtn, 125}, {_Wood, 15}, {_Road, 15},
    {_City, 25},  {_Sea, 15},  {_Arprt, 4}, {_Port, 1},  {_Brdg, 0},
    {_Shoal, 0},  {_Base, 15}, {_Pipe, 0},  {_Silo, 8},  {_Reef, 0},
};

const struct tileWeight forestTiles[] = {
    {_Plain, 15}, {_River, 0}, {_Mtn, 25},  {_Wood, 155}, {_Road, 15},
    {_City, 25},  {_Sea, 15},  {_Arprt, 4}, {_Port, 1},   {_Brdg, 0},
    {_Shoal, 0},  {_Base, 15}, {_Pipe, 0},  {_Silo, 4},   {_Reef, 0},
};

const struct tileWeight industrialTiles[] = {
    {_Plain, 15}, {_River, 0}, {_Mtn, 15},   {_Wood, 15}, {_Road, 155},
    {_City, 25},  {_Sea, 15},  {_Arprt, 6},  {_Port, 1},  {_Brdg, 80},
    {_Shoal, 10}, {_Base, 20}, {_Pipe, 100}, {_Silo, 25}, {_Reef, 0},
};
const struct tileWeight pipeTiles[] = {
    {_Plain, 15}, {_River, 0}, {_Mtn, 15},   {_Wood, 15}, {_Road, 60},
    {_City, 25},  {_Sea, 15},  {_Arprt, 4},  {_Port, 1},  {_Brdg, 40},
    {_Shoal, 10}, {_Base, 15}, {_Pipe, 255}, {_Silo, 12}, {_Reef, 0},
};
const struct tileWeight waterTiles[] = {
    {_Plain, 15},  {_River, 0}, {_Mtn, 15},  {_Wood, 15}, {_Road, 20},
    {_City, 25},   {_Sea, 155}, {_Arprt, 6}, {_Port, 8},  {_Brdg, 80},
    {_Shoal, 155}, {_Base, 15}, {_Pipe, 20}, {_Silo, 6},  {_Reef, 10},
};
const struct tileWeight riverTiles[] = {
    {_Plain, 15}, {_River, 255}, {_Mtn, 15},  {_Wood, 15}, {_Road, 20},
    {_City, 25},  {_Sea, 0},     {_Arprt, 6}, {_Port, 2},  {_Brdg, 80},
    {_Shoal, 45}, {_Base, 15},   {_Pipe, 20}, {_Silo, 6},  {_Reef, 0},
};

// extern const u32 gTileBank[];
const struct tileWeight *const gTileBank[] = {
    defaultTiles, mountainousTiles, forestTiles, industrialTiles,
    pipeTiles,    waterTiles,       riverTiles};

extern u8 Unk_200B007;

#define Plain2 0x84 >> 2
#define Plain3 0xC >> 2
#define Plain4 0x10C >> 2
extern void MakeProperty(int id, int ix, int iy);
// 8800f8e
void AddPropertyByID(int id, int ix, int iy, int map_size_x, u16 data[]) {
  if (gActiveMap->Surplus > 55) {
    return;
  }
  gActiveMap->Surplus++;
  data[(iy * map_size_x) + ix] = id;
}

void MakeSomeTile(int ix, int iy, int tile, int map_size_x, u16 data[]) {
  gActiveMap->SelectedTile = tile; // needed
  // Unk_200B036 = tiles[i].tile; // ??

  gActiveMap->SelectedTileX = ix;
  gActiveMap->SelectedTileY = iy;
  // MakeTile();

  switch (tile) {
  case _Plain: {
    MakeTileSimple(ix, iy, Plain);
    break;
  }
  case _River: {
    MakeRiver(ix, iy);
    break;
  }
  case _Mtn: {
    MakeMountain(ix, iy, 1);
    // MakeTile();
    break;
  }
  case _Wood: {
    // MakeTileSimple(ix, iy, Forest);
    MakeTile();
    break;
  }
  case _Road: {
    MakeRoad(ix, iy); // destroys properties
    break;
  }
  case _Sea: {
    // MakeTileSimple(ix, iy, Sea);
    MakeSeaSafest(ix, iy); // destroys properties
    break;
  }
  case _Brdg: {
    // MakeBridge(ix, iy, 1);
    MakeTileSimple(ix, iy, BridgeH + (Mod(Mod(ix, iy), 2) << 1));
    break;
  }
  case _Shoal: {
    // MakeSeaSafest(ix, iy); // destroys properties
    MakeTile();
    // MakeShoal(ix, iy, 1);
    break;
  }
  case _Pipe: {
    MakePipe(ix, iy);
    break;
  }
  case _Seam: {
    MakeSeam(ix, iy, 1);
    break;
  }
  case _Reef: {
    // MakeTileSimple(ix, iy, Reef);
    MakeReefSafe(ix, iy, 1);
    break;
  }
  case _City: {
    AddPropertyByID(City, ix, iy, map_size_x, data);
    // MakeProperty(0, ix, iy);
    break;
  }
  case _Base: {
    AddPropertyByID(Base, ix, iy, map_size_x, data);
    // MakeProperty(1, ix, iy);
    break;
  }
  case _Arprt: {
    AddPropertyByID(Airport, ix, iy, map_size_x, data);
    // MakeProperty(2, ix, iy);
    break;
  }
  case _Port: {
    AddPropertyByID(Port, ix, iy, map_size_x, data);
    // MakeProperty(3, ix, iy);
    break;
  }
  case _Silo: {
    AddPropertyByID(Silo, ix, iy, map_size_x, data);
    // MakeProperty(4, ix, iy);
    break;
  }
  default: {
    MakeTile(); // doesn't seem to work for properties atm
  }
  }
  // MakeTile(); // doesn't seem to work for properties atm
  //    data[(iy * map_size_x) + ix] = tile;

  // data[(iy * map_size_x) + ix] = tiles[i].tile;
}

/*
void memcpy(void *s1, const void *s2, size_t n) {
  for (int i = 0; i < n; ++i) {
    s1 = s2;
  }
}*/
extern int GetNumberOfPlayers(void);
int ValidCoord(int x, int y, int sizeX, int sizeY) {
  if (x < 0 || y < 0) {
    return false;
  }
  if (x >= sizeX || y >= sizeY) {
    return false;
  }
  return true;
}

int SetDataIfValidCoord(int x, int y, int sizeX, int sizeY, int tile,
                        u16 *data) {
  if (ValidCoord(x, y, sizeX, sizeY)) {
    data[((y)*sizeX) + (x)] = tile;
    return 1;
  }
  return 0;
}

void PlaceHQAndBase(int x, int y, int sizeX, int sizeY, int faction, u16 *data,
                    int numberOfBases) {

  if (ValidCoord(x - 1, y, sizeX, sizeY)) {
    MakeRoad(x - 1, y);
  }
  if (ValidCoord(x + 1, y, sizeX, sizeY)) {
    MakeRoad(x + 1, y);
  }
  if (ValidCoord(x, y - 1, sizeX, sizeY)) {
    MakeRoad(x, y - 1);
  }
  if (ValidCoord(x, y + 1, sizeX, sizeY)) {
    MakeRoad(x, y + 1);
  }
  data[(y * sizeX) + x] = HQ_OS + (faction * 5);

  int tile = BASE_OS + (faction * 5);
  numberOfBases -= SetDataIfValidCoord(x - 1, y, sizeX, sizeY, tile, data);

  if (numberOfBases <= 0) {
    return;
  }
  numberOfBases -= SetDataIfValidCoord(x + 1, y, sizeX, sizeY, tile, data);
  if (numberOfBases <= 0) {
    return;
  }
  numberOfBases -= SetDataIfValidCoord(x, y - 1, sizeX, sizeY, tile, data);
  if (numberOfBases <= 0) {
    return;
  }
  numberOfBases -= SetDataIfValidCoord(x, y + 1, sizeX, sizeY, tile, data);
}
extern void SetSelectedTile(int);

const u8 WigglyLineTypes[] = {
    _Road, _River, _River, _Pipe, _Sea, 0,
};

void GenerateMap(struct Map_Struct *dst, struct ChHeader *head, int chID) {
  if (!ShouldMapBeRandomized()) {
    return;
  }
  if ((int)gActiveMap < 0) {
    gActiveMap = (void *)0x200B000;
  }
  u32 activeMap = (int)gActiveMap;

  u8 someData[0x40];
  for (int i = 0; i < 0x40; ++i) {
    someData[i] = Unk_200B000[i] + 1;
    Unk_200B000[i] = 0;
  }

  int cID = gCh;
  gCh = chID;
  // dst->x = 30;
  // dst->y = 30;
  head->x = dst->x;
  head->y = dst->y;
  u8 map_size_x = dst->x;
  u8 map_size_y = dst->y;
  int numberOfPlayers = GetNumberOfPlayers();
  int defaultTile = Plain;
  u16 *data;

  // gActiveMap->SelectedTile = _Plain;
  // struct activeMap map = *gActiveMap;
  for (int iy = 0; iy < map_size_y; iy++) {
    for (int ix = 0; ix < map_size_x; ix++) {
      mapTileData[(iy * map_size_x) + ix] =
          defaultTile; // make everything the default
      // tile
      // map.SelectedTileX = ix;
      // map.SelectedTileY = iy;
      // MakeTile();
    }
  }

  // Unk_200B007 = 0;
  // gActiveMap->SelectedTile = _Plain;
  // SelectedTile_3A = 0;
  // PreviousTile_3C = 9;
  // Surplus = 0; // at 0x800C468 it checks if this is <= 0x3B
  gActiveMap->Surplus = 0;
  gActiveMap->factionProperties = 0;

  // int pathLength = 15;
  // int amountOfPaths = map_size_y * map_size_x;
  struct Vec2u p1, p2, p3, p4, q;
  int qMaxX = map_size_x >> 1;
  int qMaxY = map_size_y >> 1;
  int qMin;
  int offset = 0;

  // draw a wiggly road, river, pipe, or sea

  q.x = HashByte_Ch(map_size_x, 2, map_size_y,
                    offset + 12); // quadrant 0, 1 as left or right
  q.y = HashByte_Ch(map_size_y, 2, map_size_x, offset + 15); // up or down
  qMin = (map_size_x >> 1) * q.x;
  p1.x = HashByte_Ch(map_size_x, qMaxX, map_size_y, offset + 4) + qMin;

  qMin = (map_size_y >> 1) * q.y;
  p1.y = HashByte_Ch(map_size_y, qMaxY, map_size_x, offset + 6) + qMin;
  int type = WigglyLineTypes[Mod(p1.x + p1.y, sizeof(WigglyLineTypes))];
  q.x = !q.x;
  q.y = !q.y; // opposite corner
  qMin = (map_size_x >> 1) * q.x;
  p2.x = HashByte_Ch(p1.y, qMaxX, gCh, offset + 7) + qMin;

  qMin = (map_size_y >> 1) * q.y;
  p2.y = HashByte_Ch(p2.x, qMaxY, gCh, offset + 9) + qMin;

#ifdef ForceType
  type = ForceType;
#endif

  if (type) {
    // make it extend to the edge of the map
    if (p1.x > p1.y) {
      if (p1.x > (map_size_x >> 1)) {
        p1.x = map_size_x - 1;
        p2.x = 0;
      } else {
        p1.y = 0;
        p2.y = map_size_y - 1;
      }
    } else {
      if (p1.y > (map_size_y >> 1)) {
        p1.y = map_size_y - 1;
        p2.y = 0;
      } else {
        p1.x = 0;
        p2.x = map_size_x - 1;
      }
    }
// now draw it
#ifdef LineSegment
    drawWigglyRoad(p1.x, p1.y, p2.x, p2.y, map_size_x, 0, 1, type);
#endif
  }

#ifdef HQs
  // draw a road and connect HQs
  q.x = HashByte_Ch(gCh, 2, gCh, offset + 5); // quadrant 0, 1 as left or
  // right
  q.y = HashByte_Ch(gCh, 2, gCh,
                    offset + 8); // up or down qMin = (map_size_x >> 1) * q.x;
  qMin = (map_size_x >> 1) * q.x;
  p1.x = HashByte_Ch(gCh, qMaxX, gCh, offset + 0) + qMin;

  qMin = (map_size_y >> 1) * q.y;
  p1.y = HashByte_Ch(p1.x, qMaxY, gCh, offset + 1) + qMin;

  q.x = !q.x;
  q.y = !q.y; // opposite corner
  qMin = (map_size_x >> 1) * q.x;
  p2.x = HashByte_Ch(p1.y, qMaxX, gCh, offset + 2) + qMin;

  qMin = (map_size_y >> 1) * q.y;
  p2.y = HashByte_Ch(p2.x, qMaxY, gCh, offset + 3) + qMin;

  drawWigglyRoad(p1.x, p1.y, p2.x, p2.y, map_size_x, 0, 1, _Road);
  p3.x = p2.x;
  p3.y = Mod(p2.y + (map_size_y >> 1), map_size_y);
  if (numberOfPlayers > 2) {
    drawWigglyRoad(p2.x, p2.y, p3.x, p3.y, map_size_x, 1, 2, _Road);
    drawWigglyRoad(p1.x, p1.y, p3.x, p3.y, map_size_x, 0, 2, _Road);
  }
  p4.x = Mod(p2.x + (map_size_x >> 1), map_size_x);
  p4.y = p2.y;
  if (numberOfPlayers > 3) {
    p3.x = Mod(p1.x + (map_size_x >> 1), map_size_x);
    p3.y = p1.y;

    drawWigglyRoad(p3.x, p3.y, p4.x, p4.y, map_size_x, 2, 3, _Road);
    drawWigglyRoad(p2.x, p2.y, p4.x, p4.y, map_size_x, 1, 3, _Road);
  }
#endif

  data = mapTileData;
#ifdef Pieces
  // gCh = cID; // remove later
  // return;

  for (int iy = 0; iy < map_size_y; iy++) {
    for (int ix = 0; ix < map_size_x; ix++) {
      // dst->data[(iy * map_size_x) + ix] = 1;
      //  if (FrequencyOfObjects_Link > NextRN_N(100)) {

      CopyMapPiece(data, ix, iy, map_size_x, map_size_y, defaultTile);
    }
  }
#endif

  const struct tileWeight *bank = gTileBank[HashByte_Ch(
      gActiveMap->Surplus, sizeof(gTileBank) >> 2, p1.x, p1.y)];
#ifdef ForceTileBank
  bank = ForceTileBank;
#endif
  // const struct tileWeight *bank = gTileBank[0];
  int totalWeight = 0;
  int size = (sizeof(defaultTiles) >> 2);
  struct tileWeight tiles[size]; // defaultTiles is pointers, so >> 2
  for (int i = 0; i < size; ++i) {
    totalWeight += bank[i].weight;
    tiles[i].tile = bank[i].tile;
    tiles[i].weight = totalWeight;
  }

#ifdef Filler
  for (int iy = map_size_y - 1; iy >= 0;
       iy--) { // fill in borders with plains, forest, or mountains
    for (int ix = map_size_x - 1; ix >= 0; ix--) {
      int tmp = data[(iy * map_size_x) + ix];
      if ((tmp == Plain) || (tmp == Plain2) || (tmp == Plain3) ||
          (tmp == Plain4)) {
        int rand = HashByte_Ch(ix, totalWeight, iy, tmp);
        int i = 0;
        for (; i < size; ++i) {
          //
          if (rand < tiles[i].weight) {
            break;
          }
        }
        // tiles[i].tile = _City;

        MakeSomeTile(ix, iy, tiles[i].tile, map_size_x, data);
      }
    }
  }
#endif

#ifdef HQs
  // for some reason MakeTile kills some properties so gotta place 'em again
  PlaceHQAndBase(p1.x, p1.y, map_size_x, map_size_y, 0, data,
                 Mod(p1.x + p1.y, 3) + 2);
  PlaceHQAndBase(p2.x, p2.y, map_size_x, map_size_y, 1, data,
                 Mod(p2.x + p2.y, 3) + 2);
  if (numberOfPlayers > 2) {
    PlaceHQAndBase(p3.x, p3.y, map_size_x, map_size_y, 2, data,
                   Mod(p3.x + p3.y, 3) + 2);
  }
  if (numberOfPlayers > 3) {
    PlaceHQAndBase(p4.x, p4.y, map_size_x, map_size_y, 3, data,
                   Mod(p4.x + p4.y, 3) + 2);
  }
#endif

  data = dst->data;
  for (int iy = 0; iy < map_size_y; iy++) { // copy into initial buffer
    for (int ix = 0; ix < map_size_x; ix++) {
      data[(iy * map_size_x) + ix] = mapTileData[(iy * map_size_x) + ix];
    }
  }
  for (int i = 0; i < 0x40; ++i) {
    Unk_200B000[i] = someData[i] - 1;
  }
  gActiveMap = (void *)activeMap;
  gCh = cID;
}

void CopyMapPiece(u16 dst[], u8 placement_x, u8 placement_y, u8 map_size_x,
                  u8 map_size_y, int defaultTile) {
  int rand = HashByte_Ch(placement_x, NumberOfMapPieces, placement_y, (int)dst);
  struct Map_Struct *T = MapPiecesTable[rand];
  // struct Map_Struct *T = MapPiecesTable[0];
  int piece_size_x = (T->x);
  int piece_size_y = (T->y);
  int exit = false; // default to false

  int border_y = placement_y;
  int border_x = placement_x;
  if ((placement_y) && ((piece_size_y > 1) ||
                        (piece_size_x > 1))) { // border of 1 tile on left/above
    border_y = placement_y - 1;
  }
  if ((placement_x) && ((piece_size_y > 1) || (piece_size_x > 1))) {
    border_x = placement_x - 1;
  }
  for (int y = 0; y <= piece_size_y + 1; y++) {
    for (int x = 0; x <= piece_size_x + 1;
         x++) { // if any tile is not the default, then immediately exit
      if (dst[(border_y + y) * map_size_x + border_x + x] != defaultTile) {
        exit = true;
      }
    }
  }
  int loc, tile;
  // this is to stop it from drawing outside the map / from one side to
  // another
  if (!(((piece_size_x + placement_x) > map_size_x) ||
        ((piece_size_y + placement_y) > map_size_y) || (exit))) {
    for (int y = 0; y < piece_size_y; y++) {
      for (int x = 0; x < piece_size_x; x++) {
        loc = ((placement_y + y) * map_size_x) + placement_x + x;
        // dst[((placement_y+y) * map_size_x) + placement_x+x] =
        // T->data[y*piece_size_x+x];
        tile = T->data[y * piece_size_x + x] >> 2;
        switch (tile) {
        case City:
        case Base:
        case Airport:
        case Port:
        case Silo: {
          gActiveMap->Surplus++;
        }
        default:
        }
        dst[loc] = tile;
      }
    }
  }
}
