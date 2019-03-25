/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#include "common_selfmadex.h"

internal inline long
Max(long a, long b)
{
    return (a > b) ? a : b;
}

internal mesh_data
ImportOBJ(const char* path, GLuint vao = 0)
{
    mesh_data Mesh = { };
    debug_file RawOBJ = Win32ReadFile(path);

    Uint32 vCount = 0;
    Uint32 vtCount = 0;
    Uint32 vnCount = 0;
    Uint32 fCount = 0;
    // TODO(dave): Optimize into one loop
    for (GLchar *c = RawOBJ.data; (c - RawOBJ.data) < RawOBJ.length;)
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
    GLfloat *v  = (GLfloat *) Win32VirtualAlloc(vCount * sizeof(GLfloat));
    GLfloat *vt = (GLfloat *) Win32VirtualAlloc((vtCount ? vtCount : 1) * sizeof(GLfloat));
    GLfloat *vn = (GLfloat *) Win32VirtualAlloc((vnCount ? vnCount : 1) * sizeof(GLfloat));
    GLuint  *f  = (GLuint  *) Win32VirtualAlloc(fCount * sizeof(GLuint));
    for (GLchar *c = RawOBJ.data; (c - RawOBJ.data) < RawOBJ.length;)
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

    Uint32 lastIndex = 0;
    Mesh.vertices = (GLfloat *) Win32VirtualAlloc(fCount * sizeof(GLfloat) * 8);
    Mesh.indices  = (GLuint  *) Win32VirtualAlloc(fCount * sizeof(GLfloat) / 3);
    for (GLuint *i = f; (i - f) < fCount; i += 3)
    {
        GLuint *k = f;
        while (!((*k == *i) && (*(k+1) == *(i+1)) && (*(k+2) == *(i+2)))) {k += 3;}
        if (k == i)
        {
            *(Mesh.indices + Mesh.nIndices++) = lastIndex++;

            Uint32 vOff  = (*(i) - 1) * 3;
            Uint32 vtOff = Max((*(i+1) - 1) * 2, 0);
            Uint32 vnOff = Max((*(i+2) - 1) * 3, 0);
            
            *(Mesh.vertices + Mesh.nVertices++) = *(v+vOff);
            *(Mesh.vertices + Mesh.nVertices++) = *(v+vOff+1);
            *(Mesh.vertices + Mesh.nVertices++) = *(v+vOff+2);
            
            *(Mesh.vertices + Mesh.nVertices++) = *(vn+vnOff);
            *(Mesh.vertices + Mesh.nVertices++) = *(vn+vnOff+1);
            *(Mesh.vertices + Mesh.nVertices++) = *(vn+vnOff+2);
            
            *(Mesh.vertices + Mesh.nVertices++) = *(vt+vtOff);
            *(Mesh.vertices + Mesh.nVertices++) = *(vt+vtOff+1);
        }
        else
        {
            *(Mesh.indices + Mesh.nIndices++) = (GLuint) (k - f) / 3;
        }
        
    }

    Win32VirtualFree(vt);
    Win32VirtualFree(vn);
    Win32VirtualFree(f);
    Win32VirtualFree(RawOBJ.data);

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }
    Mesh.vao = vao;
    glBindVertexArray(Mesh.vao);
    
    glGenBuffers(2, Mesh.bo);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh.bo[VERTEX_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, Mesh.nVertices * sizeof(GLfloat), Mesh.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh.bo[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Mesh.nIndices * sizeof(GLuint), Mesh.indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(6 * sizeof(GLfloat)));    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    Win32VirtualFree(Mesh.vertices);
    Win32VirtualFree(Mesh.indices);
    return Mesh;
}

internal GLuint
ReadAndCompileShader(const char* path, GLenum type)
{
    debug_file Source = Win32ReadFile(path);
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &Source.data, (GLint*) &Source.length);
    Win32VirtualFree(Source.data);
    glCompileShader(shader);

    int Compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &Compiled);
    if (!Compiled)
    {
        int LogSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &LogSize);
        GLchar *Log = new GLchar[LogSize];
        glGetShaderInfoLog(shader, LogSize, 0, Log);

        ThrowErrorAndExit("Shader '%s' could not be compiled:\n%s", path, Log);
    }

    return shader;
}


internal void
Render(mesh_data m)
{
    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.nIndices, GL_UNSIGNED_INT, 0);
}
