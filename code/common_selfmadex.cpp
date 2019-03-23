/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#include "common_selfmadex.h"

internal GLuint
ReadAndCompileShader(const char* path, GLenum type)
{
    shader_source Source = { };
    
    std::ifstream stream;
    stream.open(path, std::ifstream::in | std::ifstream::ate);
    if (stream.is_open())
    {
        Source.length = (Uint32)stream.tellg();
        if (Source.length > 0)
        {
            stream.seekg(0);
            Source.data = new GLchar[Source.length];
            stream.read(Source.data, Source.length);
            stream.close();
        }
        else ThrowErrorAndExit("[ERROR]");
    }
    else
    {
        // TODO(dave): Logging
        ThrowErrorAndExit("[ERROR]");
    }
    
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &Source.data, (GLint*) &Source.length);
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
