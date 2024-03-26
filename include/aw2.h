#define PUREFUNC __attribute__((pure))
#define ARMFUNC __attribute__((target("arm")))
int Div(int a, int b) PUREFUNC;
int Mod(int a, int b) PUREFUNC;
int DivArm(int b, int a) PUREFUNC;

#define true 1 
#define false 0

#define STRUCT_PAD(from, to) unsigned char _pad_##from[(to) - (from)]

#include <stddef.h>
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t iptr;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t uptr;
typedef i8 fi8;
typedef i16 fi16;
typedef u8 fu8;
typedef u16 fu16;
typedef int8_t    s8;
typedef int16_t    s16;

struct Vec2i
{
    i16 x, y;
};