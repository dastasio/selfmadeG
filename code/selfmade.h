#if !defined(COMMON_SELFMADEX_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#include "selfmade_platform.h"
#include "selfmade_math.h"

#if SELFX_SLOW
#define Assert(x) if(!x) {*(int *)0 = 0;}
#else
#define Assert(x)
#endif

#if SELFX_INTERNAL
#define ThrowErrorAndExit(error, ...) { printf("[ERROR]" ## error, __VA_ARGS__); std::cin.ignore(); exit(EXIT_FAILURE); }
#define ThrowError(error, ...) { printf("[ERROR]" ## error, __VA_ARGS__); }
#else
#define ThrowErrorAndExit(error, ...)
#define ThrowError(error, ...)
#endif

#define BUFFER_OFFSET(x) ( (void *) 0)

struct memory_block
{
    bool32 IsInitialized;

    uint64 StorageSize;
    void *Storage;
};

struct mesh_data
{
    GLfloat *vertices;
    GLuint  *indices;
    GLuint   nIndices;
    GLuint   nVertices;
    GLuint   vao;
    GLuint   bo[2];
};

struct shader_source
{
    uint32  length;
    GLbyte *data;
};


#define COMMON_SELFMADEX_H
#endif
