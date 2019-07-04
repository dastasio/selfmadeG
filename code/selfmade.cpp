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

internal debug_obj_file
ReadOBJ(debug_file *RawObj, memory_pool *Pool, bool IsTriangulated, bool FillMissingAttrs)
{
    debug_obj_file File = {};
    uint32 vCount = 0;
    uint32 vtCount = 0;
    uint32 vnCount = 0;
    uint32 fCount = 0;
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
    File.ComponentsPerVertex = 3;
    if(vnCount)
    {
        File.ComponentsPerVertex += 3;
    }
    if(vtCount)
    {
        File.ComponentsPerVertex += 2;
    }
    uint32 VerticesPerFace = IsTriangulated ? 3 : 4;
    if((FillMissingAttrs) || (File.ComponentsPerVertex == 8))
    {
        fCount *= VerticesPerFace*3;
    }
    else if(File.ComponentsPerVertex == 6)
    {
        fCount *= VerticesPerFace*2;
    }
    else if(File.ComponentsPerVertex == 3)
    {
        fCount *= VerticesPerFace;
    }
    
    File.DataOffsetInMemoryPool = Pool->Base + Pool->Used;
    real32 *v  = PushArray(Pool, vCount, GLfloat);
    real32 *vt = PushArray(Pool, (((!vtCount) && (FillMissingAttrs)) ? 1 : vtCount), GLfloat);
    real32 *vn = PushArray(Pool, (((!vnCount) && (FillMissingAttrs)) ? 1 : vnCount), GLfloat);
    uint32  *f  = PushArray(Pool, fCount, GLuint);
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
                    if(*c == '/')
                    {
                        *f = strtoul( (const char *)++c, &c, 10);
                    }
                    if(FillMissingAttrs || (*f != 0))
                    {
                        f++;//*(f++) = 0;
                    }
                    if(*c == '/')
                    {
                        *f = strtoul( (const char *)++c, &c, 10);
                    }
                    if(FillMissingAttrs || (*f != 0))
                    {
                        f++;//*(f++) = 0;
                    }
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
    File.VP = v;
    File.VN = vn;
    File.VT = vt;
    File.F = f;
    File.IndexCount = fCount;
    File.VertexCount = vCount;

    return(File);
}

internal mesh_data
MeshFromOBJ(debug_file *RawObj, memory_pool *Pool, GLuint vao = 0)
{
    mesh_data Mesh = { };
    debug_obj_file ObjFile = ReadOBJ(RawObj, Pool, 1, 1);

    uint32 LastIndex = 0;
    GLfloat *Vertices = PushArray(Pool, ObjFile.IndexCount*8, GLfloat);
    GLuint *Indices  = PushArray(Pool, ObjFile.IndexCount/3, GLuint);
    for (GLuint *i = ObjFile.F; (i - ObjFile.F) < ObjFile.IndexCount; i += 3)
    {
        GLuint *k = ObjFile.F;
        while (!((*k == *i) && (*(k+1) == *(i+1)) && (*(k+2) == *(i+2)))) {k += 3;}
        if (k == i)
        {
            *(Indices + Mesh.nIndices++) = LastIndex++;

            uint32 vOff  = (*(i) - 1) * 3;
            uint32 vtOff = Max((int32)(*(i+1) - 1) * 2, 0);
            uint32 vnOff = Max((int32)(*(i+2) - 1) * 3, 0);
            
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VP+vOff);
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VP+vOff+1);
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VP+vOff+2);
            
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VN+vnOff);
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VN+vnOff+1);
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VN+vnOff+2);
            
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VT+vtOff);
            *(Vertices + Mesh.nVertices++) = *(ObjFile.VT+vtOff+1);
        }
        else
        {
            *(Indices + Mesh.nIndices++) = *(Indices + Mesh.nIndices - ((i-k)/3) );
        }
        
    }

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

    PopMemoryPoolToIndex(Pool, ObjFile.DataOffsetInMemoryPool);
    return Mesh;
}

internal rigid_body
RigidBodyFromOBJ(debug_file *RawObj, memory_pool *Pool)
{
    debug_obj_file ObjFile = ReadOBJ(RawObj, Pool, 0, 0);
    Assert((ObjFile.ComponentsPerVertex == 6));

    rigid_body RigidBody = {};
    RigidBody.VP = ObjFile.VP;
    RigidBody.VN = ObjFile.VN;
    RigidBody.Faces = ObjFile.F;
    RigidBody.FCount = ObjFile.IndexCount/8;
    RigidBody.VCount = ObjFile.VertexCount/3;

    return(RigidBody);
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

internal V3 *
FarthestPointInDirection(V3 Direction, V3 *Points, uint32 Count)
{
    real32 HighestDistance = Inner(Points[0], Direction);
    V3 *MostDistantPoint = Points;
    for(uint32 n = 1;
        n < Count;
        ++n)
    {
        V3 *Point = &Points[n];
        real32 Distance = Inner(*Point, Direction);
        if(Distance > HighestDistance)
        {
            HighestDistance = Distance;
            MostDistantPoint = Point;
        }
    }
    return(MostDistantPoint);
}
inline V3
GJKMaxMinkowski(V3 *PointsA, uint32 CountA, V3 PositionA,
                V3 *PointsB, uint32 CountB, V3 PositionB,
                V3 Direction)
{
    V3 *MaxA = FarthestPointInDirection(Direction, PointsA, CountA);
    V3 *MaxB = FarthestPointInDirection(-Direction, PointsB, CountB);
    V3 Result = (*MaxA + PositionA) - (*MaxB + PositionB);
    return(Result);
}

inline bool32
GJKProcessSimplex(V3 *Simplex, uint32 *SimplexSize, V3 *Direction)
{
    bool32 ContainsOrigin = false;

#define SAME_DIRECTION(A, B) ((Inner((A), (B)) > 0))
    uint32 A = (*SimplexSize)-1;
    uint32 B = (*SimplexSize)-2;
    uint32 C = (*SimplexSize)-3;
    uint32 D = (*SimplexSize)-4;
    if(*SimplexSize == 2)
    {
        // Root(||A||^2) + Root(||B||^2) = Root(||AB||^2)
        // (||AB||^2 - ||A||^2 - ||B||^2)^2 = 4 ||A||^2 ||B||^2
        V3 AB = Simplex[B] - Simplex[A];
        real32 Asq = LengthSq(Simplex[A]);
        real32 Bsq = LengthSq(Simplex[B]);
        real32 ABsq = LengthSq(AB);
        if(Square(ABsq - Asq - Bsq) == (4.f*Asq*Bsq))
        {
            ContainsOrigin = true;
        }
    }
    else if(*SimplexSize == 3)
    {
        V3 AB = Simplex[B] - Simplex[A];
        V3 CA = Simplex[A] - Simplex[C];
        V3 BC = Simplex[C] - Simplex[B];
        V3 Normal = Cross(AB, BC);
        if(Inner(-Simplex[A], Normal) == 0.f)
        {
            real32 AO_AB = Inner(-Simplex[A], AB);
            real32 BO_BC = Inner(-Simplex[B], BC);
            real32 CO_CA = Inner(-Simplex[C], CA);
            if((AO_AB >= 0) && (AO_AB < 1.f) &&
               (BO_BC >= 0) && (BO_BC < 1.f) &&
               (CO_CA >= 0) && (CO_CA < 1.f))
            {
                ContainsOrigin = true;
            }
        }
    }
    else if(*SimplexSize == 4)
    {
        V3 AB = Simplex[B] - Simplex[A];
        V3 AC = Simplex[C] - Simplex[A];
        V3 AD = Simplex[D] - Simplex[A];
        V3 BC = Simplex[C] - Simplex[B];
        V3 BD = Simplex[D] - Simplex[B];
        V3 CD = Simplex[D] - Simplex[C];

        V3 ABxAC = Cross(AB, AC);
        V3 BAxBD = Cross(-AB, BD);
        V3 CBxCD = Cross(-BC, CD);
        V3 DCxDA = Cross(-CD, -AD);
        if(Inner(AD, ABxAC) < 0)
        {
            ABxAC = -ABxAC;
        }
        if(Inner(BC, BAxBD) < 0)
        {
            BAxBD = -BAxBD;
        }
        if(Inner(-AC, CBxCD) < 0)
        {
            CBxCD = -CBxCD;
        }
        if(Inner(-BD, DCxDA) < 0)
        {
            DCxDA = -DCxDA;
        }

        if((Inner(-Simplex[A], ABxAC) >= 0) &&
           (Inner(-Simplex[B], BAxBD) >= 0) &&
           (Inner(-Simplex[C], CBxCD) >= 0) &&
           (Inner(-Simplex[D], DCxDA) >= 0))
        {
            ContainsOrigin = true;
        }
    }

    if(!ContainsOrigin)
    {
        V3 AO = -Simplex[A];
        switch(*SimplexSize)
        {
            case 2:
            {
                V3 AB = Simplex[B] - Simplex[A];
                if(SAME_DIRECTION(AB, AO))
                {
                    *Direction = Cross(Cross(AB, AO), AB);
                }
                //else
                //{
                //    Simplex[B] = Simplex[A];
                //    *SimplexSize = 1;
                //    *Direction = AO;
                //}
            } break;

            case 4:
            {
                real32 LenA = LengthSq(Simplex[A]);
                real32 LenB = LengthSq(Simplex[B]);
                real32 LenC = LengthSq(Simplex[C]);
                real32 LenD = LengthSq(Simplex[D]);
                if((LenA + LenC + LenD) < (LenA + LenB + LenD))
                {
                    if((LenA + LenC + LenD) < (LenA + LenB + LenC))
                    {
                        Simplex[D] = Simplex[D];
                        Simplex[C] = Simplex[C];
                        Simplex[B] = Simplex[A];
                    }
                    else
                    {
                        Simplex[D] = Simplex[C];
                        Simplex[C] = Simplex[B];
                        Simplex[B] = Simplex[A];
                    }
                }
                else
                {
                    if((LenA + LenB + LenD) < (LenA + LenB + LenC))
                    {
                        Simplex[D] = Simplex[D];
                        Simplex[C] = Simplex[B];
                        Simplex[B] = Simplex[A];
                    }
                    else
                    {
                        Simplex[D] = Simplex[C];
                        Simplex[C] = Simplex[B];
                        Simplex[B] = Simplex[A];
                    }
                }
                *SimplexSize = 3;
                A = (*SimplexSize)-1;
                B = (*SimplexSize)-2;
                C = (*SimplexSize)-3;
                //D = (*SimplexSize)-4;
                // NOTE(dave): Falls through
            } 

            case 3:
            {
                V3 AB = Simplex[B] - Simplex[A];
                V3 AC = Simplex[C] - Simplex[A];
                V3 ABC = Cross(AB, AC);
                bool CheckAB = false;
                if(SAME_DIRECTION(Cross(ABC, AC), AO))
                {
                    if(SAME_DIRECTION(AC, AO))
                    {
                        //Simplex[C] = Simplex[C];
                        Simplex[B] = Simplex[A];
                        *SimplexSize = 2;
                        *Direction = Cross(Cross(AC, AO), AC);
                    }
                    else
                    {
                        CheckAB = true;
                    }
                }
                else
                {
                    if(SAME_DIRECTION(Cross(AB, ABC), AO))
                    {
                    }
                    else
                    {
                        if(SAME_DIRECTION(ABC, AO))
                        {
                            //Simplex[C] = Simplex[C];
                            //Simplex[B] = Simplex[B];
                            //Simplex[A] = Simplex[A];
                            //*SimplexSize = 3;
                            *Direction = ABC;
                        }
                        else
                        {
                            Simplex[3] = Simplex[C];
                            Simplex[C] = Simplex[B];
                            Simplex[B] = Simplex[3];
                            //Simplex[A] = Simplex[A];
                            //*SimplexSize = 3;
                            *Direction = -ABC;
                        }
                    }
                }

                if(CheckAB)
                {
                    if(SAME_DIRECTION(AB, AO))
                    {
                        Simplex[C] = Simplex[B];
                        Simplex[B] = Simplex[A];
                        *SimplexSize = 2;
                        *Direction = Cross(Cross(AB, AO), AB);
                    }
                    else
                    {
                        Simplex[C] = Simplex[A];
                        *SimplexSize = 1;
                        *Direction = AO;
                    }
                }
            } break;

        }
    }

    return(ContainsOrigin);
}

internal bool32
CollisionAfterMovement(mesh_data *A, mesh_data *B)
{
    bool32 Collides = -1;

    V3 SearchDirection = A->dPCurrentStep;
    V3 Simplex[4];
    uint32 SimplexSize = 0;

    V3 *APoints = (V3 *)&A->RigidBody.VP[0];
    V3 *BPoints = (V3 *)&B->RigidBody.VP[0];
    V3 NewPoint = GJKMaxMinkowski(APoints, A->RigidBody.VCount, A->Position + A->dPCurrentStep,
                                  BPoints, B->RigidBody.VCount, B->Position, SearchDirection);//BVerts, 4, B->Position, SearchDirection);
    Simplex[SimplexSize++] = NewPoint;
    SearchDirection = -NewPoint;
    while(Collides < 0)
    {
        NewPoint = GJKMaxMinkowski(APoints, A->RigidBody.VCount, A->Position + A->dPCurrentStep,
                                   BPoints, B->RigidBody.VCount, B->Position, SearchDirection);//BVerts, 4, B->Position, SearchDirection);
        if(Inner(NewPoint, SearchDirection) < 0)
        {
            Collides = 0;
        }
        else
        {
            for(uint32 s = 0;
                (s < SimplexSize) && (Collides < 0);
                ++s)
            {
                if(Simplex[s] == NewPoint)
                {
                    Collides = false;
                }
            }
            if(Collides < 0)
            {
                Simplex[SimplexSize++] = NewPoint;
                if(GJKProcessSimplex(Simplex, &SimplexSize, &SearchDirection))
                {
                    Collides = true;
                }
            }
        }
    }
    
    if(Collides)
    {
        real32 LeastDistance = (real32)0x2FFFFFFF;
        V3 *NormalOfClosestFace = 0;
        uint32 *FaceArrayEnd = B->RigidBody.Faces + B->RigidBody.FCount*8;
        for(uint32 *Face = B->RigidBody.Faces;
            Face < FaceArrayEnd;
            Face += 8)
        {
            V3 *Normal = (V3 *)(B->RigidBody.VN + (Face[1]-1)*3);
            if(Inner(*Normal, A->dPCurrentStep) < 0)
            {
                Assert((Face[1] == Face[3]) && (Face[1] == Face[5]) && (Face[1] == Face[7]));
                V3 *vA = (V3 *)(B->RigidBody.VP + (Face[0]-1)*3);
                V3 *vB = (V3 *)(B->RigidBody.VP + (Face[2]-1)*3);
                V3 *vC = (V3 *)(B->RigidBody.VP + (Face[4]-1)*3);
                V3 *vD = (V3 *)(B->RigidBody.VP + (Face[6]-1)*3);
                real32 LenA = LengthSq(*vA - (A->Position + A->dPCurrentStep));
                real32 LenB = LengthSq(*vB - (A->Position + A->dPCurrentStep));
                real32 LenC = LengthSq(*vC - (A->Position + A->dPCurrentStep));
                real32 LenD = LengthSq(*vD - (A->Position + A->dPCurrentStep));
                real32 CurrentDistance = (LenA + LenB + LenC + LenD);
                if(CurrentDistance < LeastDistance)
                {
                    LeastDistance = CurrentDistance;
                    NormalOfClosestFace = Normal;
                }
            }
        }

        A->dPCurrentStep -= *NormalOfClosestFace*Inner(A->dPCurrentStep, *NormalOfClosestFace);
    }
    ThrowError("GJK: %s\n\n", Collides ? "Colliding" : "Not Colliding");
    return(Collides);
}

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
        debug_file SceneBodyObj = Memory->SDLPlatformReadEntireFile("test.obj");
        debug_file PlayerObj = Memory->SDLPlatformReadEntireFile("player.obj");
        debug_file PlayerBodyObj = Memory->SDLPlatformReadEntireFile("player_body.obj");
        debug_file LightObj = Memory->SDLPlatformReadEntireFile("light.obj");
        State->Scene =  MeshFromOBJ(&SceneObj, &State->MemoryPool);
        State->Player = MeshFromOBJ(&PlayerObj, &State->MemoryPool);
        State->Player.Position = {0.f, 0.5f, 0.f};
        //State->Player.Position = {-30.f, 0.5f, -29.f};

        //State->Scene.nColliders = 1;
        //State->Scene.Colliders = PushArray(&State->MemoryPool, State->Scene.nColliders, collider);
        State->Scene.RigidBody = RigidBodyFromOBJ(&SceneBodyObj, &State->MemoryPool);
        //State->Scene.Colliders[0].Center = {-30.1332f, 0.635283f, -29.2653f};
        //State->Scene.Colliders[0].Radius = {6.5669f, 0.635283f, 7.303f};
        //State->Player.nColliders = 1;
        //State->Player.Colliders = PushArray(&State->MemoryPool, State->Player.nColliders, collider);
        State->Player.RigidBody = RigidBodyFromOBJ(&PlayerBodyObj, &State->MemoryPool);
        //State->Player.Colliders[0].Center = {0.000000f, 0.999445f, 0.000000f};
        //State->Player.Colliders[0].Radius = {0.258501f, 0.999445f, 0.313644f};

        State->MainLight.Mesh = MeshFromOBJ(&LightObj, &State->MemoryPool);
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

    if (Input->RX.Value)
    {
        State->Camera.Yaw -= 0.02f*PI*Input->RX.Value;
    }
    if (Input->RY.Value)
    {
        State->Camera.Pitch += 0.02f*PI*Input->RY.Value;
    }
    State->Camera.Position.X = Sin32(State->Camera.Yaw);
    State->Camera.Position.Y = Sin32(State->Camera.Pitch);
    State->Camera.Position.Z = Cos32(State->Camera.Yaw)*Cos32(State->Camera.Pitch);
    State->Camera.Position = (State->Camera.Target + 
                              Normalize(State->Camera.Position)*State->Camera.DistanceFromTarget);

    V3 ddPVector = State->Camera.Space.V*Input->LX.Value;
    ddPVector += -State->Camera.Space.N*Input->LY.Value;
    ddPVector.Y = 0.f;//-9.81f;
    //if((Input->A.Value) && (State->Player.dPosition.Y == 0.f))
    //{
    //    ddPVector.Y += 30*9.81f;
    //}
    State->Player.ddPosition = 40.f*ddPVector;

    while(*SecondsToAdvance >= PHYSICS_TIMESTEP) // 1/120
    {
        State->Player.dPosition = State->Player.dPosition + State->Player.ddPosition*PHYSICS_TIMESTEP;
        //if((State->Player.Position.Y <= 0.f) && (State->Player.dPosition.Y < 0.f))
        //{
        //    State->Player.dPosition.Y = 0;
        //    State->Player.ddPosition.Y = 0;
        //}
        V3 Friction = 7.f*State->Player.dPosition;
        Friction.Y = 0;
        State->Player.ddPosition -= Friction;
        V3 Movement = (State->Player.dPosition*PHYSICS_TIMESTEP +
                       0.5f*State->Player.ddPosition*Square(PHYSICS_TIMESTEP));
        State->Player.dPCurrentStep = Movement;

        bool32 Collision = CollisionAfterMovement(&State->Player, &State->Scene);
        if(Collision)
        {
            Movement.X = 0.f;
            Movement.Y = 0.f;
            Movement.Z = 0.f;
        }
//        if(Collision & 0x1)
//        {
//            Movement.X = 0;
//        }
//        if(Collision & 0x2)
//        {
//            Movement.Y = 0;
//        }
//        if(Collision & 0x4)
//        {
//            Movement.Z = 0;
//        }
        State->Player.Position += State->Player.dPCurrentStep;
        State->Camera.Position += State->Player.dPCurrentStep;
        State->Camera.Target +=   State->Player.dPCurrentStep;
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
