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

#define BUFFER_OFFSET(x) ((void *)(x)) 

struct memory_pool
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;
};

internal void
InitializeMemoryPool(memory_pool *Pool, uint8 *Base, memory_index Size)
{
    Pool->Base = Base;
    Pool->Size = Size;
    Pool->Used = 0;
}

#define PushStruct(Pool, Type) (Type *)PushIntoMemoryPool_((Pool), sizeof(Type))
#define PushArray(Pool, Count, Type) (Type *)PushIntoMemoryPool_((Pool), (Count)*sizeof(Type))
inline void *
PushIntoMemoryPool_(memory_pool *Pool, memory_index Size)
{
    Assert(((Pool->Used + Size) <= Pool->Size));
    void *Result = (uint8 *)Pool->Base + Pool->Used;
    Pool->Used += Size;
    return Result;
}
inline void
PopMemoryPoolToIndex(memory_pool *Pool, uint8 *TargetIndex)
{
    Assert(((TargetIndex >= Pool->Base) && (TargetIndex <= (Pool->Base + Pool->Used))));
    for(uint8 *Index = Pool->Base + Pool->Used;
        Index >= TargetIndex;
        --Index)
    {
        *Index = 0;
    }
}

struct mesh_data
{
    //GLfloat *vertices;
    //GLuint  *indices;
    GLuint   nIndices;
    GLuint   nVertices;
    GLuint   vao;
    GLuint   bo[2];

    vec3 Scale;
    vec3 Rotation;
    vec3 Position;
};

struct light_data
{
    mesh_data Mesh;
    vec3 Color;
};

struct camera_data
{
    vec3 Position;
    vec3 Target;
    vec3 Up;

    camera_space Space;
};

struct game_state
{
    uint32 ShadingProgram;
    uint32 LightProgram;
    mesh_data Scene;
    mesh_data Player;
    light_data MainLight;
    camera_data Camera;

    memory_pool MemoryPool;
};


#define COMMON_SELFMADEX_H
#endif
