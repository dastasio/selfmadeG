#if !defined(SELFMADE_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

struct debug_file
{
    uint32  Length;
    GLchar *Data;
};

#define SDL_PLATFORM_READ_ENTIRE_FILE(name) debug_file name(char *Filename)
typedef SDL_PLATFORM_READ_ENTIRE_FILE(sdl_platform_read_entire_file);

struct memory_block
{
    bool32 IsInitialized;

    uint64 StorageSize;
    void *Storage;

    sdl_platform_read_entire_file *SDLPlatformReadEntireFile;
};

#define STICK_DEADZONE 16384
#define STICK_VALUE(x) ((((x) >= -STICK_DEADZONE) && ((x) <= STICK_DEADZONE)) ? 0.f : ((real32)(x) / 32768.f))
struct input_button_state
{
    real32 Value;
};

struct input
{
    input_button_state Up;
    input_button_state Down;
    input_button_state Left;
    input_button_state Right;

    input_button_state A;
    input_button_state B;
    input_button_state X;
    input_button_state Y;

    input_button_state LB;
    input_button_state RB;

    input_button_state LX;
    input_button_state LY;
    input_button_state RX;
    input_button_state RY;
    input_button_state LT;
    input_button_state RT;
};

#ifdef __cplusplus
}
#endif

#define SELFMADE_PLATFORM_H
#endif
