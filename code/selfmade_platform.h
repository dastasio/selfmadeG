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

#ifdef __cplusplus
}
#endif

#define SELFMADE_PLATFORM_H
#endif