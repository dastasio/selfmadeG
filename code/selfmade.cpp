/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#include "selfmade.h"

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
#if 0
struct initialization_memory
{
    debug_file VertexShader;
    debug_file FragmentShader;

};

internal void
InitializeMemory(memory_block Memory)
{
}
#endif
internal void
Render(memory_block *Memory)
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
        debug_file Obj = Memory->SDLPlatformReadEntireFile("scene.obj");
        State->Mesh = ImportOBJ(&Obj, &State->MemoryPool);
        State->Camera.Position = {0.f, 0.f, -0.5f};
        State->Camera.Target = {};
        State->Camera.Up = {0.f, 1.f, 0.f};

        glUseProgram(State->ShadingProgram);
        glDisable(GL_CULL_FACE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.15f, 0.3f, 0.3f, 1);

        Memory->IsInitialized = true;
    }
    camera_data *Cam = &State->Camera;
    const uint8 *KeyboardState = SDL_GetKeyboardState(0);
    if(KeyboardState[SDL_SCANCODE_W])
    {
        Cam->Position += 0.05f*Cam->Space.N;
    }
    if (KeyboardState[SDL_SCANCODE_A])
    {
        Cam->Position -= 0.05f*Cam->Space.V;
    }
    if (KeyboardState[SDL_SCANCODE_S])
    {
        Cam->Position -= 0.05f*Cam->Space.N;
    }
    if (KeyboardState[SDL_SCANCODE_D])
    {
        Cam->Position += 0.05f*Cam->Space.V;
    }
    if (KeyboardState[SDL_SCANCODE_LSHIFT])
    {
        Cam->Target += 0.05f*Cam->Space.U;
        Cam->Position += 0.05f*Cam->Space.U;
    }
    if (KeyboardState[SDL_SCANCODE_LCTRL])
    {
        Cam->Target -= 0.05f*Cam->Space.U;
        Cam->Position -= 0.05f*Cam->Space.U;
    }

    mat4 CameraMatrix = ComputeCameraSpace(&State->Camera.Space,
                                           State->Camera.Position,
                                           State->Camera.Target,
                                           State->Camera.Up);
    mat4 PerspectiveMatrix = PerspectiveProjection(90.f, 1024.f/720.f, 0.1f, 100.f);
    vec3 test = UnitVectorFromVector3({1.f, 1.f, 0.f});

    mat4 TransformationMatrix = PerspectiveMatrix * CameraMatrix;
    glUniformMatrix4fv(0, 1, false, &TransformationMatrix[0][0]);
    glUniform3f(1, Cam->Position.x, Cam->Position.y, Cam->Position.z);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(State->Mesh.vao);
    glDrawElements(GL_TRIANGLES, State->Mesh.nIndices, GL_UNSIGNED_INT, 0);
}
