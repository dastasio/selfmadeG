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

//#define 2PI 6.28318530718f
#define  PI 3.14159265358f
#define PI2 1.57079632679f
#define PHYSICS_TIMESTEP 0.008333333333333f

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

struct debug_obj_object
{
    real32 *VStart;
    real32 *VOnePastEnd;
    uint32 *F;
    uint32 IndexCount;
};

struct debug_obj_file
{
    real32 *VP;
    real32 *VN;
    real32 *VT;

    debug_obj_object *Objects;
    uint32 ObjectCount;

    uint32 VertexCount;
    uint32 ComponentsPerVertex;
    
    uint8 *DataOffsetInMemoryPool;
};

struct collision_box
{
    // NOTE(dave): 
    // Each face has 4 position indices and 4 normal indices
    real32 *VertexRangeLowerIndex;
    real32 *VertexRangeOnePastUpperIndex;
    uint32 *Faces;
    uint32 FaceCount;
};

struct collision_set
{
    real32 *VP;
    real32 *VN;
    uint32 TotalVertexCount;

    uint32 ColliderCount;
    collision_box *Colliders;
};

struct mesh_data
{
    //GLfloat *vertices;
    //GLuint  *indices;
    GLuint   nIndices;
    GLuint   nVertices;
    GLuint   vao;
    GLuint   bo[2];

    collision_set RigidBody;
    V3 Scale;
    V3 Rotation;
    V3 Position;

    V3 dPosition;     // m/s
    V3 ddPosition;
    V3 dPCurrentStep;
};

struct light_data
{
    mesh_data Mesh;
    V3 Color;
};

struct camera_data
{
    V3 Position;
    V3 Target;
    V3 Up;

    real32 Pitch;
    real32 Yaw;
    real32 DistanceFromTarget;
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
