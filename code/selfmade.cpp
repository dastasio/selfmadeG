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
    uint32 oCount = 0;
    // TODO(dave): Optimize into one loop
    for (char *c = RawObj->Data; (c - RawObj->Data) < RawObj->Length;)
    {
        switch (*c++)
        {
            // TODO(dave): Add multiple mesh support
            case 'o': 
            {
                oCount++;
                while(*c++ != '\n') {}
            } break;
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
    debug_obj_object *o = PushArray(Pool, oCount, debug_obj_object);
    File.Objects = o--;
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
                    o->IndexCount++;
                    if(*c == '/')
                    {
                        *f = strtoul( (const char *)++c, &c, 10);
                    }
                    if(FillMissingAttrs || (*f != 0))
                    {
                        f++;//*(f++) = 0;
                        o->IndexCount++;
                    }
                    if(*c == '/')
                    {
                        *f = strtoul( (const char *)++c, &c, 10);
                    }
                    if(FillMissingAttrs || (*f != 0))
                    {
                        f++;//*(f++) = 0;
                        o->IndexCount++;
                    }
                }
                c++;
            } break;
            case 'o':
            {
                if(o >= File.Objects)
                {
                    o->VOnePastEnd = v;
                }
                o++;
                o->VStart = v;
                o->F = f;
                while(*c++ != '\n') {}
            } break;
            default: while(*c++ != '\n') {} break;
        }
    }
    o->VOnePastEnd = v;
    v  -= vCount;
    vt -= vtCount;
    vn -= vnCount;
    f -= fCount;
    debug_obj_object *EndObject = o;
    EndObject++;
    o -= (oCount - 1);
    File.VP = v;
    File.VN = vn;
    File.VT = vt;
    File.ObjectCount = oCount;
    File.VertexCount = vCount;

    uint32 Sum = 0;
    for(debug_obj_object *Obj = o;
        Obj < EndObject;
        ++Obj)
    {
        Sum += Obj->IndexCount;
    }
    Assert((Sum == fCount));
    return(File);
}

internal mesh_data
MeshFromOBJ(debug_file *RawObj, memory_pool *Pool, GLuint vao = 0)
{
    mesh_data Mesh = { };
    debug_obj_file ObjFile = ReadOBJ(RawObj, Pool, 1, 1);

    // TODO(dave): Support multiple objects
    uint32 LastIndex = 0;
    GLfloat *Vertices = PushArray(Pool, ObjFile.Objects->IndexCount*8, GLfloat);
    GLuint *Indices  = PushArray(Pool, ObjFile.Objects->IndexCount/3, GLuint);
    for (GLuint *i = ObjFile.Objects->F; (i - ObjFile.Objects->F) < ObjFile.Objects->IndexCount; i += 3)
    {
        GLuint *k = ObjFile.Objects->F;
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

internal void
CollidersFromOBJIntoMesh(debug_file *RawObj, memory_pool *Pool, mesh_data *Mesh)
{
    debug_obj_file ObjFile = ReadOBJ(RawObj, Pool, 0, 0);
    Assert((ObjFile.ComponentsPerVertex == 6));
    collision_box *Boxes = PushArray(Pool, ObjFile.ObjectCount, collision_box);

    Mesh->RigidBody.VP = ObjFile.VP;
    Mesh->RigidBody.VN = ObjFile.VN;
    Mesh->RigidBody.TotalVertexCount = ObjFile.VertexCount/3;
    for(uint32 BoxIndex = 0;
        BoxIndex < ObjFile.ObjectCount;
        ++BoxIndex)
    {
        collision_box *Box = &Boxes[BoxIndex];
        Box->VertexRangeLowerIndex = ObjFile.Objects[BoxIndex].VStart;
        Box->VertexRangeOnePastUpperIndex = ObjFile.Objects[BoxIndex].VOnePastEnd;
        Box->Faces = ObjFile.Objects[BoxIndex].F;
        Box->FaceCount = ObjFile.Objects[BoxIndex].IndexCount/8;
    }
    Mesh->RigidBody.Colliders = Boxes;
    Mesh->RigidBody.ColliderCount = ObjFile.ObjectCount;
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

#define LINE_MAX_NUMBER 100
global_variable V3 LineBuffer[LINE_MAX_NUMBER][3];
global_variable uint32 LineCount = 0;

internal void
DrawLine(V3 A, V3 B, V3 Color)
{
    //glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(4);
    glUniform3fv(2, 1, &A.E[0]);
    glUniform3fv(3, 1, &B.E[0]);
    glUniform3f(4, Color.X, Color.Y, Color.Z);
    glDrawArrays(GL_LINES, 0, 2);
    glEnable(GL_DEPTH_TEST);
}

internal void
DrawPoint(V3 P, V3 Color)
{
    //glDisable(GL_DEPTH_TEST);
    glPointSize(10);
    glUniform3fv(2, 1, &P.E[0]);
    glUniform3f(4, Color.X, Color.Y, Color.Z);
    glDrawArrays(GL_POINTS, 0, 1);
    glEnable(GL_DEPTH_TEST);
}

internal V3 *
FarthestPointInDirection(V3 Direction, V3 *PointsStart, V3 *PointsOnePastEnd)
{
    real32 HighestDistance = Inner(PointsStart[0], Direction);
    V3 *MostDistantPoint = PointsStart;
    for(V3 *Point = &PointsStart[1];
        Point < PointsOnePastEnd;
        ++Point)
    {
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
GJKMaxMinkowski(V3 *PointsAStart, V3 *PointsAOnePastEnd, V3 PositionA,
                V3 *PointsBStart, V3 *PointsBOnePastEnd, V3 PositionB,
                V3 Direction)
{
    V3 *MaxA = FarthestPointInDirection(Direction, PointsAStart, PointsAOnePastEnd);
    V3 *MaxB = FarthestPointInDirection(-Direction, PointsBStart, PointsBOnePastEnd);
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

internal V3
CollisionAfterMovement(mesh_data *A, mesh_data *B)
{
    bool32 Collides = -1;
    V3 Intersection = {};

    //ThrowError("Position: (%f; %f; %f)\n", A->Position.X, A->Position.Y, A->Position.Z);
    for(uint32 IndexA = 0;
        (IndexA < A->RigidBody.ColliderCount) && (Collides != 1);
        ++IndexA)
    {
        for(uint32 IndexB = 0;
            (IndexB < B->RigidBody.ColliderCount) && (Collides != 1);
            ++IndexB)
        {
            V3 SearchDirection = A->dPCurrentStep;
            V3 Simplex[4];
            uint32 SimplexSize = 0;
            Collides = -1;

            V3 NewPoint = GJKMaxMinkowski((V3 *) A->RigidBody.Colliders[IndexA].VertexRangeLowerIndex,
                                          (V3 *) A->RigidBody.Colliders[IndexA].VertexRangeOnePastUpperIndex,
                                          A->Position + A->dPCurrentStep,
                                          (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeLowerIndex,
                                          (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeOnePastUpperIndex,
                                          B->Position, SearchDirection);
            Simplex[SimplexSize++] = NewPoint;
            SearchDirection = -NewPoint;
            V3 OldPoint = {};
            while(Collides < 0)
            {
                NewPoint = GJKMaxMinkowski((V3 *) A->RigidBody.Colliders[IndexA].VertexRangeLowerIndex,
                                           (V3 *) A->RigidBody.Colliders[IndexA].VertexRangeOnePastUpperIndex,
                                           A->Position + A->dPCurrentStep,
                                           (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeLowerIndex,
                                           (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeOnePastUpperIndex,
                                           B->Position, SearchDirection);
                if(Inner(NewPoint, SearchDirection) < 0)
                {
                    Collides = 0;
                }
                else
                {
                    if(OldPoint == NewPoint)
                    {
                        Collides = false;
                    }
                    else
                    {
                        for(uint32 s = 0;
                            (s < SimplexSize) && (Collides < 0);
                            ++s)
                        {
                            if(Absolute(Inner(NewPoint, SearchDirection) - Inner(Simplex[s], SearchDirection)) < 0.01f)
                            {
                                Collides = false;
                            }
                        }
                    }
                    if(Collides < 0)
                    {
                        Simplex[SimplexSize++] = NewPoint;
                        if(GJKProcessSimplex(Simplex, &SimplexSize, &SearchDirection))
                        {
                            Collides = true;
                        }
                        OldPoint = NewPoint;
                    }
                }
            }

            if(Collides)
            {
#if 1
                Assert((SimplexSize == 4));
                V3 Polytope[10][3] = {
                    {Simplex[3], Simplex[2], Simplex[1]},
                    {Simplex[3], Simplex[1], Simplex[0]},
                    {Simplex[3], Simplex[2], Simplex[0]},
                    {Simplex[2], Simplex[1], Simplex[0]}
                };
                uint32 PolytopeSize = 4;

                LineCount = 0;
                real32 LineColor = 0.f;
                int32 MaxIterations = 10;
                while(!Intersection && (MaxIterations-- >= 0))
                {
                    real32 LeastDistanceFromOrigin = (real32)0x2FFFFFFF;
                    V3 *LeastDistantFace = 0;
                    V3 NormalOfSelectedFace = {};
                    for(uint32 FaceIndex = 0;
                        FaceIndex < PolytopeSize;
                        ++FaceIndex)
                    {
                        V3 *Face = (V3 *)&Polytope[FaceIndex];
                        V3 Normal = Normalize(Cross(Face[1] - Face[0], Face[2] - Face[0]));
                        ThrowError("Normal: (%f; %f; %f)\n", Normal.X, Normal.Y, Normal.Z);
                        real32 DistanceFromOrigin = Inner(Normal, -Face[0]);
                        if(DistanceFromOrigin > 0)
                        {
                            Normal = -Normal;
                        }
                        else
                        {
                            DistanceFromOrigin *= -1.f;
                        }
                        if(DistanceFromOrigin < LeastDistanceFromOrigin)
                        {
                            LeastDistanceFromOrigin = DistanceFromOrigin;
                            LeastDistantFace = Face;
                            NormalOfSelectedFace = Normal;
                        }

                        Assert(LineCount < ArraySize(LineBuffer));
                        //LineBuffer[LineCount][0] = Face[0];
                        //LineBuffer[LineCount][1] = Face[1];
                        //LineBuffer[LineCount++][2] = {LineColor, LineColor, 0.f};
                        //LineBuffer[LineCount][0] = Face[0];
                        //LineBuffer[LineCount][1] = Face[2];
                        //LineBuffer[LineCount++][2] = {LineColor, LineColor, 0.f};
                        //LineBuffer[LineCount][0] = Face[1];
                        //LineBuffer[LineCount][1] = Face[2];
                        //LineBuffer[LineCount++][2] = {LineColor, LineColor, 0.f};
                    }
                    //LineBuffer[LineCount][0] = LeastDistantFace[0];
                    //LineBuffer[LineCount][1] = LeastDistantFace[1];
                    //LineBuffer[LineCount++][2] = {0.f, LineColor, LineColor};
                    //LineBuffer[LineCount][0] = LeastDistantFace[0];
                    //LineBuffer[LineCount][1] = LeastDistantFace[2];
                    //LineBuffer[LineCount++][2] = {0.f, LineColor, LineColor};
                    //LineBuffer[LineCount][0] = LeastDistantFace[1];
                    //LineBuffer[LineCount][1] = LeastDistantFace[2];
                    //LineBuffer[LineCount++][2] = {0.f, LineColor, LineColor};

                    NewPoint = GJKMaxMinkowski((V3 *) A->RigidBody.Colliders[IndexA].VertexRangeLowerIndex,
                                               (V3 *) A->RigidBody.Colliders[IndexA].VertexRangeOnePastUpperIndex,
                                               A->Position + A->dPCurrentStep,
                                               (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeLowerIndex,
                                               (V3 *) B->RigidBody.Colliders[IndexB].VertexRangeOnePastUpperIndex,
                                               B->Position, NormalOfSelectedFace);
                    LineColor += 0.5f;

                    bool IsAlreadyInPolytope = false;
                    for(uint32 FaceIndex = 0;
                        (FaceIndex < PolytopeSize) && !IsAlreadyInPolytope;
                        ++FaceIndex)
                    {
                        V3 *Face = (V3 *)&Polytope[FaceIndex];
                        IsAlreadyInPolytope = (Face[0] == NewPoint);
                        IsAlreadyInPolytope = IsAlreadyInPolytope || (Face[1] == NewPoint);
                        IsAlreadyInPolytope = IsAlreadyInPolytope || (Face[2] == NewPoint);
                    }
                    if(
                       (Inner(NewPoint, NormalOfSelectedFace) - LeastDistanceFromOrigin) < 0.001f)
                    {
                        Intersection = LeastDistanceFromOrigin*NormalOfSelectedFace;
                    }
                    //if(IsAlreadyInPolytope)
                    //{
                    //    // TODO(dave): return currently selected face
                    //    Intersection = LeastDistanceFromOrigin*NormalOfSelectedFace;
                    //}
                    else
                    {
                        uint32 FacesToCull[3] = {};
                        uint32 CullCount = 0;
                        for(uint32 FaceIndex = 0;
                            FaceIndex < PolytopeSize;
                            ++FaceIndex)
                        {
                            V3 *Face = (V3 *)&Polytope[FaceIndex];
                            V3 Normal = Normalize(Cross(Face[1] - Face[0], Face[2] - Face[0]));
                            if(SAME_DIRECTION(Normal, -Face[0]))
                            {
                                Normal = -Normal;
                            }
                            if(SAME_DIRECTION(Normal, NewPoint))
                            {
                                FacesToCull[CullCount++] = FaceIndex;
                            }
                        }
                        switch(CullCount)
                        {
                            case 0:
                            {
                                V3 FreePoints[10];
                                uint32 FreePointsCount = 0;
                                for(uint32 FaceIndex0 = 0;
                                    FaceIndex0 < PolytopeSize;
                                    ++FaceIndex0)
                                {
                                    V3 *Face0 = (V3 *)&Polytope[FaceIndex0];
                                    V3 Edges[3][2] = {
                                        {Face0[0], Face0[1]},
                                        {Face0[0], Face0[2]},
                                        {Face0[1], Face0[2]}
                                    };
                                    V3 *Edge = Edges[0];
                                    while(Edge <= Edges[2])
                                    {
                                        bool EdgeIsShared = false;
                                        for(uint32 FaceIndex1 = 0;
                                            (FaceIndex1 < PolytopeSize) && !EdgeIsShared;
                                            ++FaceIndex1)
                                        {
                                            if(FaceIndex1 != FaceIndex0)
                                            {
                                                V3 *Face1 = (V3 *)&Polytope[FaceIndex1];
                                                bool FirstEdgeVertexIsShared = ((Edge[0] == Face1[0]) ||
                                                                                (Edge[0] == Face1[1]) ||
                                                                                (Edge[0] == Face1[2]));
                                                bool SecondEdgeVertexIsShared = ((Edge[1] == Face1[0]) ||
                                                                                 (Edge[1] == Face1[1]) ||
                                                                                 (Edge[1] == Face1[2]));
                                                EdgeIsShared = (FirstEdgeVertexIsShared &&
                                                                SecondEdgeVertexIsShared);
                                            }
                                        }
                                        if(!EdgeIsShared)
                                        {
                                            FreePoints[FreePointsCount++] = Edge[0];
                                            FreePoints[FreePointsCount++] = Edge[1];
                                        }
                                        Edge += 2;
                                    }
                                }
                                Assert((FreePointsCount < ArraySize(FreePoints)));
                                for(uint32 FreePointIndex = 0;
                                    FreePointIndex < FreePointsCount;
                                    )
                                {
                                    //Assert(NewPoint != FreePoints[FreePointIndex]);
                                    Polytope[PolytopeSize][0] = FreePoints[FreePointIndex++];
                                    //Assert(NewPoint != FreePoints[FreePointIndex]);
                                    Polytope[PolytopeSize][1] = FreePoints[FreePointIndex++];
                                    Polytope[PolytopeSize++][2] = NewPoint;
                                }
                            } break;

                            case 1:
                            {
                                V3 TempFace[3] = {
                                    Polytope[FacesToCull[0]][0],
                                    Polytope[FacesToCull[0]][1],
                                    Polytope[FacesToCull[0]][2]
                                };
                                Polytope[FacesToCull[0]][0] = TempFace[0];
                                Polytope[FacesToCull[0]][1] = TempFace[1];
                                Polytope[FacesToCull[0]][2] = NewPoint;
                                Polytope[PolytopeSize][0] = TempFace[0];
                                Polytope[PolytopeSize][1] = TempFace[2];
                                Polytope[PolytopeSize++][2] = NewPoint;
                                Polytope[PolytopeSize][0] = TempFace[1];
                                Polytope[PolytopeSize][1] = TempFace[2];
                                Polytope[PolytopeSize++][2] = NewPoint;
                            } break;

                            case 2:
                            {
                                Polytope[PolytopeSize][0] = Polytope[FacesToCull[0]][0];
                                Polytope[PolytopeSize][1] = Polytope[FacesToCull[0]][2];
                                Polytope[PolytopeSize++][2] = NewPoint;
                                Polytope[FacesToCull[0]][2] = NewPoint;

                                Polytope[PolytopeSize][0] = Polytope[FacesToCull[1]][0];
                                Polytope[PolytopeSize][1] = Polytope[FacesToCull[1]][2];
                                Polytope[PolytopeSize++][2] = NewPoint;
                                Polytope[FacesToCull[1]][2] = NewPoint;
                            } break;

                            case 3:
                            {
                                uint32 CommonPointsFace0[2] = {4, 4};
                                uint32 CommonPointsFace1[2] = {4, 4};
                                uint32 CommonPointFace0 = 4;
                                uint32 CommonPointFace1 = 4;
                                uint32 CommonPointFace2 = 4;
                                for(uint32 i = 0;
                                    (i < 3) && (CommonPointsFace1[1] == 4);
                                    ++i)
                                {
                                    for(uint32 j = 0;
                                        (j < 3) && (CommonPointsFace1[1] == 4);
                                        ++j)
                                    {
                                        if(Polytope[FacesToCull[0]][i] == Polytope[FacesToCull[1]][j])
                                        {
                                            if(CommonPointsFace0[0])
                                            {
                                                CommonPointsFace0[1] = i;
                                                CommonPointsFace1[1] = j;
                                            }
                                            else
                                            {
                                                CommonPointsFace0[0] = i;
                                                CommonPointsFace1[0] = j;
                                            }
                                        }
                                    }
                                }
                                for(uint32 k = 0;
                                    k < 3;
                                    ++k)
                                {
                                    if(Polytope[FacesToCull[2]][k] == Polytope[FacesToCull[0]][CommonPointsFace0[0]])
                                    {
                                        CommonPointFace0 = CommonPointsFace0[0];
                                        CommonPointFace1 = CommonPointsFace1[0];
                                        CommonPointFace2 = k;
                                    }
                                    else if(Polytope[FacesToCull[2]][k] == Polytope[FacesToCull[0]][CommonPointsFace0[1]])
                                    {
                                        CommonPointFace0 = CommonPointsFace0[1];
                                        CommonPointFace1 = CommonPointsFace1[1];
                                        CommonPointFace2 = k;
                                    }
                                }
                                Polytope[FacesToCull[0]][CommonPointFace0] = NewPoint;
                                Polytope[FacesToCull[1]][CommonPointFace1] = NewPoint;
                                Polytope[FacesToCull[2]][CommonPointFace2] = NewPoint;
                            } break;

                            default:
                            {
                                V3 FreePoints[20];
                                uint32 FreePointsCount = 0;
                                for(uint32 FaceIndex0 = 0;
                                    FaceIndex0 < CullCount;
                                    ++FaceIndex0)
                                {
                                    V3 *Face0 = (V3 *)&Polytope[FacesToCull[FaceIndex0]];
                                    V3 Edges[3][2] = {
                                        {Face0[0], Face0[1]},
                                        {Face0[0], Face0[2]},
                                        {Face0[1], Face0[2]}
                                    };
                                    V3 *Edge = Edges[0];
                                    while(Edge <= Edges[2])
                                    {
                                        bool EdgeIsShared = false;
                                        for(uint32 FaceIndex1 = 0;
                                            (FaceIndex1 < CullCount) && !EdgeIsShared;
                                            ++FaceIndex1)
                                        {
                                            if(FaceIndex1 != FaceIndex0)
                                            {
                                                V3 *Face1 = (V3 *)&Polytope[FacesToCull[FaceIndex1]];
                                                bool FirstEdgeVertexIsShared = ((Edge[0] == Face1[0]) ||
                                                                                (Edge[0] == Face1[1]) ||
                                                                                (Edge[0] == Face1[2]));
                                                bool SecondEdgeVertexIsShared = ((Edge[1] == Face1[0]) ||
                                                                                 (Edge[1] == Face1[1]) ||
                                                                                 (Edge[1] == Face1[2]));
                                                EdgeIsShared = (FirstEdgeVertexIsShared &&
                                                                SecondEdgeVertexIsShared);
                                            }
                                        }
                                        if(!EdgeIsShared)
                                        {
                                            FreePoints[FreePointsCount++] = Edge[0];
                                            FreePoints[FreePointsCount++] = Edge[1];
                                        }
                                        Edge += 2;
                                    }
                                }
                                Assert((FreePointsCount < ArraySize(FreePoints)));
                                for(uint32 FreePointIndex = 0;
                                    FreePointIndex < FreePointsCount;
                                    )
                                {
                                    uint32 PolytopeIndex = ((FreePointIndex < CullCount) ?
                                                            FacesToCull[FreePointIndex] :
                                                            PolytopeSize++);
                                    //Assert(NewPoint != FreePoints[FreePointIndex]);
                                    Polytope[PolytopeIndex][0] = FreePoints[FreePointIndex++];
                                    //Assert(NewPoint != FreePoints[FreePointIndex]);
                                    Polytope[PolytopeIndex][1] = FreePoints[FreePointIndex++];
                                    Polytope[PolytopeIndex][2] = NewPoint;
                                }
                            } break;
                        }
                    }

                }
#else
                    real32 LeastDistance = (real32)0x2FFFFFFF;
                    V3 *NormalOfClosestFace = 0;
                    uint32 *FaceArrayEnd = B->RigidBody.Colliders[IndexB].Faces + B->RigidBody.Colliders[IndexB].FaceCount*8;
                    for(uint32 *Face = B->RigidBody.Colliders[IndexB].Faces;
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
                            if(CurrentDistance*CurrentDistance < LeastDistance)
                            {
                                LeastDistance = CurrentDistance;
                                NormalOfClosestFace = Normal;
                            }
                        }
                    }

                    A->dPCurrentStep -= *NormalOfClosestFace*Inner(A->dPCurrentStep, *NormalOfClosestFace);
                    Collides = -1;
#endif
            }
        }
    }
    //ThrowError("GJK: %s\n\n", Collides ? "Colliding" : "Not Colliding");
    //ThrowError("Intersection: (%f; %f; %f)\n", Intersection.X, Intersection.Y, Intersection.Z);
    return(Intersection);
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
        State->LineProgram = CompileShaderProgram(Memory->SDLPlatformReadEntireFile,
                                                  "shaders/linev.glsl",
                                                  "shaders/linef.glsl");
        debug_file SceneObj = Memory->SDLPlatformReadEntireFile("scene.obj");
        debug_file SceneBodyObj = Memory->SDLPlatformReadEntireFile("scene_body.obj");
        debug_file PlayerObj = Memory->SDLPlatformReadEntireFile("player.obj");
        debug_file PlayerBodyObj = Memory->SDLPlatformReadEntireFile("player_body.obj");
        debug_file LightObj = Memory->SDLPlatformReadEntireFile("light.obj");
        State->Scene =  MeshFromOBJ(&SceneObj, &State->MemoryPool);
        State->Player = MeshFromOBJ(&PlayerObj, &State->MemoryPool);
        //State->Player.Position = {0.f, 0.5f, 0.f};
        State->Player.Position = {7.47f, 0.5f, -22.25f};//{-30.f, 0.5f, -29.f};

        //State->Scene.nColliders = 1;
        //State->Scene.Colliders = PushArray(&State->MemoryPool, State->Scene.nColliders, collider);
        CollidersFromOBJIntoMesh(&SceneBodyObj, &State->MemoryPool, &State->Scene);
        //State->Scene.Colliders[0].Center = {-30.1332f, 0.635283f, -29.2653f};
        //State->Scene.Colliders[0].Radius = {6.5669f, 0.635283f, 7.303f};
        //State->Player.nColliders = 1;
        //State->Player.Colliders = PushArray(&State->MemoryPool, State->Player.nColliders, collider);
        CollidersFromOBJIntoMesh(&PlayerBodyObj, &State->MemoryPool, &State->Player);
        //State->Player.Colliders[0].Center = {0.000000f, 0.999445f, 0.000000f};
        //State->Player.Colliders[0].Radius = {0.258501f, 0.999445f, 0.313644f};

        State->MainLight.Mesh = MeshFromOBJ(&LightObj, &State->MemoryPool);
        State->MainLight.Mesh.Position = {0.f, 25.f, 0.f};
        State->MainLight.Color = {1.f, 1.f, 1.f};
        State->Camera.Target = {7.47f, 2.2f, -22.25f};//{0.f, 1.7f, 0.f};
        //State->Camera.Target += State->Player.Position;
        State->Camera.Up = {0.f, 1.f, 0.f};
        State->Camera.DistanceFromTarget = 5.f;
        State->Camera.Yaw = -PI/4.f;
        State->Camera.Pitch = PI/6.f;
        State->Camera.Position = {4.7f, 4.56f, -19.7f};

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

        GLfloat LineData[] = {0.f, 1.f};
        uint32 LineVBO;
        glGenVertexArrays(1, &State->LineVAO);
        glBindVertexArray(State->LineVAO);

        glGenBuffers(1, &LineVBO);
        glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(LineData), LineData, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glEnableVertexAttribArray(0);

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
    ddPVector.Y = 0.f;
    State->Player.ddPosition = 40.f*ddPVector;
    //State->Player.ddPosition.Y = -9.81f*(1.f-Input->A.Value) + 4.f*9.81f*Input->A.Value;

    V3 TotalCollision = {};
    ThrowError("Starting Frame Calculation\n");
    while(*SecondsToAdvance >= PHYSICS_TIMESTEP) // 1/120
    {
        ThrowError("PositionA: (%f; %f; %f)\n", State->Player.Position.X, State->Player.Position.Y, State->Player.Position.Z);
        State->Player.dPosition = State->Player.dPosition + State->Player.ddPosition*PHYSICS_TIMESTEP;
        //if((State->Player.Position.Y <= 0.f) && (State->Player.dPosition.Y < 0.f))
        //{
        //    State->Player.dPosition.Y = 0;
        //    State->Player.ddPosition.Y = 0;
        //}
        V3 Friction = 7.f*State->Player.dPosition;
        Friction.Y = 0;
        State->Player.ddPosition -= Friction;
        State->Player.dPCurrentStep = (State->Player.dPosition*PHYSICS_TIMESTEP +
                       0.5f*State->Player.ddPosition*Square(PHYSICS_TIMESTEP));

        ThrowError("PositionB: (%f; %f; %f)\n", State->Player.Position.X, State->Player.Position.Y, State->Player.Position.Z);
        V3 Collision = CollisionAfterMovement(&State->Player, &State->Scene);
        ThrowError("CollisionA: (%f; %f; %f)\n", Collision.X, Collision.Y, Collision.Z);

        if(Collision)
        {
            TotalCollision += Collision;
            real32 Modulus = Length(Collision);
            Collision.Y = 0;
            Collision = Modulus*Normalize(Collision);
            State->Player.dPCurrentStep -= Collision;
            ThrowError("CollisionB: (%f; %f; %f)\n", Collision.X, Collision.Y, Collision.Z);
        }
        State->Player.Position += State->Player.dPCurrentStep;
        State->Camera.Position += State->Player.dPCurrentStep;
        State->Camera.Target +=   State->Player.dPCurrentStep;
        *SecondsToAdvance -= PHYSICS_TIMESTEP;
        ThrowError("PositionC: (%f; %f; %f)\n\n", State->Player.Position.X, State->Player.Position.Y, State->Player.Position.Z);
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

    glUseProgram(State->LineProgram);
    glBindVertexArray(State->LineVAO);
    glUniformMatrix4fv(1, 1, false, &ScreenSpaceTransform.E[0][0]);
    DrawLine(State->Player.Position, State->Player.Position + State->Player.dPCurrentStep*50.f, {0.7f, 0.3f, 0.5f});
    //DrawLine(State->Player.Position, State->Player.Position + TotalCollision*50.f, {0.3f, 0.5f, 0.7f});

    DrawPoint({}, {0.f, 0.f, 0.f});
    for(uint32 Line = 0;
        Line < LineCount;
        ++Line)
    {
        DrawLine(LineBuffer[Line][0], LineBuffer[Line][1], LineBuffer[Line][2]);
    }
}
