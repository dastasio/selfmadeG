/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#include "selfmade.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

internal inline long
Max(long a, long b)
{
    return (a > b) ? a : b;
}

internal mesh_data
ImportOBJ(debug_file *RawObj, memory_pool *Pool, GLuint vao = 0)
{
    mesh_data Mesh = { };

    Uint32 vCount = 0;
    Uint32 vtCount = 0;
    Uint32 vnCount = 0;
    Uint32 fCount = 0;
    // TODO(dave): Optimize into one loop
    for (char *c = RawObj->Data; (c - RawObj->Data) < RawObj->Length;)
    {
        switch (*c++)
        {
            // TODO(dave): Add multiple mesh support
            case 'o': while(*c++ != '\n') {} break;
            case 'f': fCount++; break;
            case 'v':
            {
                switch (*(c++))
                {
                    case 'n': vnCount++; break;
                    case 't': vtCount++; break;
                    case ' ':  vCount++; break;
                }
            } break;
            default: while(*c++ != '\n') {} break;
        }
    }

    vCount *= 3;
    vtCount *= 2;
    vnCount *= 3;
    fCount *= 9;
    
    uint8 *PoolIndexBeforeAllocations = Pool->Base + Pool->Used;
    GLfloat *v  = PushArray(Pool, vCount, GLfloat); //(GLfloat *) Win32VirtualAlloc(vCount * sizeof(GLfloat));
    GLfloat *vt = PushArray(Pool, (vtCount ? vtCount : 1), GLfloat); //(GLfloat *) Win32VirtualAlloc((vtCount ? vtCount : 1) * sizeof(GLfloat));
    GLfloat *vn = PushArray(Pool, (vnCount ? vnCount : 1), GLfloat); //(GLfloat *) Win32VirtualAlloc((vnCount ? vnCount : 1) * sizeof(GLfloat));
    GLuint  *f  = PushArray(Pool, fCount, GLuint); //(GLuint  *) Win32VirtualAlloc(fCount * sizeof(GLuint));
    for (GLchar *c = RawObj->Data; (c - RawObj->Data) < RawObj->Length;)
    {
        switch (*c++)
        {
            case 'v':
            {
                GLfloat **selected = 0;
                switch (*(c++))
                {
                    case 'n': selected = &vn; break;
                    case 't': selected = &vt; break;
                    case ' ': selected = &v; break;
                }
                while (*c != '\n')
                {
                    **selected = strtof( (const char *)c, &c);
                    (*selected)++;
                }
                c++;
            } break;
            case 'f':
            {
                while (*c != '\n')
                {
                    *(f++) = strtoul( (const char *)c, &c, 10);
                    if (*c == '/') *(f++) = strtoul( (const char *)++c, &c, 10);
                    else *(f++) = 0;
                    if (*c == '/') *(f++) = strtoul( (const char *)++c, &c, 10);
                    else *(f++) = 0;
                }
                c++;
            } break;
            default: while(*c++ != '\n') {} break;
        }
    }
    v  = v  -  vCount;
    vt = vt - vtCount;
    vn = vn - vnCount;
    f  = f  -  fCount;
    // NOTE(dave): not needed because Win32's VirtualAlloc already clears to 0
    //if (!vtCount) { *vt = 0; *(vt+1) = 0; }
    //if (!vnCount) { *vn = 0; *(vn+1) = 0; *(vn+2) = 0; }

    uint32 LastIndex = 0;
    GLfloat *Vertices = PushArray(Pool, fCount*8, GLfloat); //(GLfloat *) Win32VirtualAlloc(fCount * sizeof(GLfloat) * 8);
    GLuint *Indices  = PushArray(Pool, fCount/3, GLuint); //(GLuint  *) Win32VirtualAlloc(fCount * sizeof(GLfloat) / 3);
    for (GLuint *i = f; (i - f) < fCount; i += 3)
    {
        GLuint *k = f;
        while (!((*k == *i) && (*(k+1) == *(i+1)) && (*(k+2) == *(i+2)))) {k += 3;}
        if (k == i)
        {
            *(Indices + Mesh.nIndices++) = LastIndex++;

            Uint32 vOff  = (*(i) - 1) * 3;
            Uint32 vtOff = Max((*(i+1) - 1) * 2, 0);
            Uint32 vnOff = Max((*(i+2) - 1) * 3, 0);
            
            *(Vertices + Mesh.nVertices++) = *(v+vOff);
            *(Vertices + Mesh.nVertices++) = *(v+vOff+1);
            *(Vertices + Mesh.nVertices++) = *(v+vOff+2);
            
            *(Vertices + Mesh.nVertices++) = *(vn+vnOff);
            *(Vertices + Mesh.nVertices++) = *(vn+vnOff+1);
            *(Vertices + Mesh.nVertices++) = *(vn+vnOff+2);
            
            *(Vertices + Mesh.nVertices++) = *(vt+vtOff);
            *(Vertices + Mesh.nVertices++) = *(vt+vtOff+1);
        }
        else
        {
            *(Indices + Mesh.nIndices++) = *(Indices + Mesh.nIndices - ((i-k)/3) );
        }
        
    }

//    Win32VirtualFree(vt);
//    Win32VirtualFree(vn);
//    Win32VirtualFree(f);
//    Win32VirtualFree(RawObj->Data);

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }
    Mesh.vao = vao;
    glBindVertexArray(Mesh.vao);
    
    glGenBuffers(2, Mesh.bo);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh.bo[VERTEX_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, Mesh.nVertices * sizeof(GLfloat), Vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh.bo[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Mesh.nIndices * sizeof(GLuint), Indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(6 * sizeof(GLfloat)));    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    PopMemoryPoolToIndex(Pool, PoolIndexBeforeAllocations);
//    Win32VirtualFree(Mesh.vertices);
//    Win32VirtualFree(Mesh.indices);
    return Mesh;
}

internal GLuint
CompileShader(debug_file *Source, char *Path, GLenum type)
{
    GLuint Shader = glCreateShader(type);
    glShaderSource(Shader, 1, &Source->Data, (GLint*) &Source->Length);
    Win32VirtualFree(Source->Data);
    glCompileShader(Shader);

    int Compiled;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Compiled);
    if (!Compiled)
    {
        int LogSize;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogSize);
        GLchar *Log = new GLchar[LogSize];
        glGetShaderInfoLog(Shader, LogSize, 0, Log);

        ThrowErrorAndExit("Shader '%s' could not be compiled:\n%s", Path, Log);
    }

    return Shader;
}

internal GLuint
CompileShaderProgram(sdl_platform_read_entire_file SDLPlatformReadEntireFile,
                     char *VertexPath, char *FragmentPath)
{
    GLuint Program;
    debug_file VertexSource = SDLPlatformReadEntireFile(VertexPath);
    debug_file FragmentSource = SDLPlatformReadEntireFile(FragmentPath);
    GLuint VShader = CompileShader(&VertexSource, VertexPath, GL_VERTEX_SHADER);
    GLuint FShader = CompileShader(&FragmentSource, FragmentPath, GL_FRAGMENT_SHADER);

    Program = glCreateProgram();
    glAttachShader(Program, VShader);
    glAttachShader(Program, FShader);
    glLinkProgram(Program);
    glDeleteShader(VShader);
    glDeleteShader(FShader);

    int Success;
    glGetProgramiv(Program, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        int LogSize;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogSize);
        GLchar *Log = new GLchar[LogSize];
        glGetProgramInfoLog(Program, LogSize, 0, Log);

        ThrowErrorAndExit("Unable to link shader program:\n%s", Log);
    }

    return Program;
}

internal bool32
CollisionAfterMovement(V3 dPosA,
                       V3 PosA, collider ColA,
                       V3 PosB, collider ColB)
{
    V3 Center1 = ColA.Center + PosA;
    V3 Center2 = ColB.Center + PosB;
    V3 DistanceBeforeMovement = Absolute(Center1 - Center2);
    Center1 += dPosA;
    V3 DistanceAfterMovement = Absolute(Center1 - Center2);
    bool32 Result = ((DistanceAfterMovement.X <= (ColA.Radius.X + ColB.Radius.X)) &&
                     (DistanceAfterMovement.Y <= (ColA.Radius.Y + ColB.Radius.Y)) &&
                     (DistanceAfterMovement.Z <= (ColA.Radius.Z + ColB.Radius.Z)));
    if(Result)
    {
        Result = (!(DistanceBeforeMovement.X <= (ColA.Radius.X + ColB.Radius.X)) |
                  !(DistanceBeforeMovement.Y <= (ColA.Radius.Y + ColB.Radius.Y)) << 1 |
                  !(DistanceBeforeMovement.Z <= (ColA.Radius.Z + ColB.Radius.Z)) << 2);
    }
    return Result;
}

// TODO(dave): Should I use fixed time steps for updates?
internal void
UpdateAndRender(memory_block *Memory, input *Input, real32 *SecondsToAdvance)
{
    Assert((sizeof(game_state) <= Memory->StorageSize));
    game_state *State = (game_state *)Memory->Storage;
    if(!Memory->IsInitialized)
    {
        InitializeMemoryPool(&State->MemoryPool,
                             (uint8 *)Memory->Storage + sizeof(game_state),
                             Memory->StorageSize - sizeof(game_state));

        State->ShadingProgram = CompileShaderProgram(Memory->SDLPlatformReadEntireFile,
                                                     "shaders/vertex.glsl",
                                                     "shaders/fragment.glsl");
        State->LightProgram = CompileShaderProgram(Memory->SDLPlatformReadEntireFile,
                                                   "shaders/lightv.glsl",
                                                   "shaders/lightf.glsl");
        debug_file SceneObj = Memory->SDLPlatformReadEntireFile("scene.obj");
        debug_file PlayerObj = Memory->SDLPlatformReadEntireFile("player.obj");
        debug_file LightObj = Memory->SDLPlatformReadEntireFile("light.obj");
        State->Scene = ImportOBJ(&SceneObj, &State->MemoryPool);
        State->Player = ImportOBJ(&PlayerObj, &State->MemoryPool);

        State->Scene.nColliders = 1;
        State->Scene.Colliders = PushArray(&State->MemoryPool, State->Scene.nColliders, collider);
        State->Scene.Colliders[0].Center = {-30.1332f, 0.635283f, -29.2653f};
        State->Scene.Colliders[0].Radius = {6.5669f, 0.635283f, 7.303f};
        State->Player.nColliders = 1;
        State->Player.Colliders = PushArray(&State->MemoryPool, State->Player.nColliders, collider);
        State->Player.Colliders[0].Center = {0.000000f, 0.999445f, 0.000000f};
        State->Player.Colliders[0].Radius = {0.258501f, 0.999445f, 0.313644f};
        //Center = {0.000000f, 0.999445f, 0.000000f};
        //Radius = {0.323305f, 0.999445f, 0.313644f};

        State->MainLight.Mesh = ImportOBJ(&LightObj, &State->MemoryPool);
        State->MainLight.Mesh.Position = {0.f, 25.f, 0.f};
        State->MainLight.Color = {1.f, 1.f, 1.f};
        State->Camera.Target = {0.f, 1.7f, 0.f};
        State->Camera.Up = {0.f, 1.f, 0.f};
        State->Camera.DistanceFromTarget = 2.f;

        //glDisable(GL_CULL_FACE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.15f, 0.3f, 0.3f, 1);
        glUseProgram(State->ShadingProgram);

        //int32 ImageWidth, ImageHeight, ImageChannels;
        //uint8 *ImageData = stbi_load("container.jpg", &ImageWidth, &ImageHeight, &ImageChannels, 0);
        //glGenTextures(1, &Texture);
        //glBindTexture(GL_TEXTURE_2D, Texture);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        //             ImageWidth, ImageHeight, 0,
        //             GL_RGB, GL_UNSIGNED_BYTE, ImageData);
        //glGenerateMipmap(GL_TEXTURE_2D);
        //stbi_image_free(ImageData);

        Memory->IsInitialized = true;
    }

    if (Input->RightAxisX.Value)
    {
        State->Camera.Yaw -= 0.02f*PI*Input->RightAxisX.Value;
    }
    if (Input->RightAxisY.Value)
    {
        State->Camera.Pitch += 0.02f*PI*Input->RightAxisY.Value;
    }
    State->Camera.Position.X = Sin32(State->Camera.Yaw);
    State->Camera.Position.Y = Sin32(State->Camera.Pitch);
    State->Camera.Position.Z = Cos32(State->Camera.Yaw)*Cos32(State->Camera.Pitch);
    State->Camera.Position = (State->Camera.Target + 
                              Normalize(State->Camera.Position)*State->Camera.DistanceFromTarget);

    V3 MoveVector = State->Camera.Space.V*Input->LeftAxisX.Value;
    MoveVector += -State->Camera.Space.N*Input->LeftAxisY.Value;
    MoveVector.Y = 0.f;
    State->Player.ddPosition = 40.f*MoveVector;

    while(*SecondsToAdvance >= PHYSICS_TIMESTEP) // 1/120
    {
        State->Player.dPosition = State->Player.dPosition + State->Player.ddPosition*PHYSICS_TIMESTEP;
        State->Player.ddPosition -= 7.0f*State->Player.dPosition;
        V3 Movement = (State->Player.dPosition*PHYSICS_TIMESTEP +
                       0.5f*State->Player.ddPosition*Square(PHYSICS_TIMESTEP));

        bool32 Collision = CollisionAfterMovement(
            Movement,
            State->Player.Position, State->Player.Colliders[0],
            State->Scene.Position, State->Scene.Colliders[0]
        );
        if(Collision & 0x1)
        {
            Movement.X = 0;
        }
        if(Collision & 0x2)
        {
            Movement.Y = 0;
        }
        if(Collision & 0x4)
        {
            Movement.Z = 0;
        }
        State->Player.Position += Movement;
        State->Camera.Position += Movement;
        State->Camera.Target += Movement;
        *SecondsToAdvance -= PHYSICS_TIMESTEP;
    }

    M4 CameraMatrix = ComputeCameraSpace(&State->Camera.Space,
                                           State->Camera.Position,
                                           State->Camera.Target,
                                           State->Camera.Up);
    M4 PerspectiveMatrix = PerspectiveProjection(90.f, 1024.f/720.f, 0.1f, 100.f);

    glUseProgram(State->ShadingProgram);
    M4 ScreenSpaceTransform = PerspectiveMatrix * CameraMatrix;
    glUniformMatrix4fv(0, 1, false, &ScreenSpaceTransform.E[0][0]);
    glUniform3fv(1, 1, &State->Camera.Position.E[0]);
    glUniform3fv(3, 1, &State->MainLight.Mesh.Position.E[0]);
    glUniform3fv(4, 1, &State->MainLight.Color.E[0]);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    M4 ModelTransform = TranslationMatrix4(State->Scene.Position);
    glUniformMatrix4fv(2, 1, false, &ModelTransform.E[0][0]);
    glBindVertexArray(State->Scene.vao);
    glDrawElements(GL_TRIANGLES, State->Scene.nIndices, GL_UNSIGNED_INT, 0);
    
    ModelTransform = TranslationMatrix4(State->Player.Position);
    glUniformMatrix4fv(2, 1, false, &ModelTransform.E[0][0]);
    glBindVertexArray(State->Player.vao);
    glDrawElements(GL_TRIANGLES, State->Player.nIndices, GL_UNSIGNED_INT, 0);

    glUseProgram(State->LightProgram);
    ModelTransform = TranslationMatrix4(State->MainLight.Mesh.Position);
    glUniformMatrix4fv(0, 1, false, &ScreenSpaceTransform.E[0][0]);
    glUniformMatrix4fv(2, 1, false, &ModelTransform.E[0][0]);
    glUniform3fv(3, 1, &State->MainLight.Color.E[0]);
    glBindVertexArray(State->MainLight.Mesh.vao);
    glDrawElements(GL_TRIANGLES, State->MainLight.Mesh.nIndices, GL_UNSIGNED_INT, 0);
}
