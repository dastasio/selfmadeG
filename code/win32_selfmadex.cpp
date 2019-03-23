/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#define SDL_MAIN_HANDLED
#include "glad.c"
#include <sdl2/SDL.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

#define VSYNC(x) SDL_GL_SetSwapInterval(x)
#define WIDTH 1024
#define HEIGHT 720

#define internal static
#define local_persist static
#define global_variable static

#include "common_selfmadex.h"
#include "common_selfmadex.cpp"

inline void
LogProgramLinking(GLuint p)
{
    int Success;
    glGetProgramiv(p, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        int LogSize;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &LogSize);
        GLchar *Log = new GLchar[LogSize];
        glGetProgramInfoLog(p, LogSize, 0, Log);

        ThrowErrorAndExit("Unable to link shader program:\n%s", Log);
    }
}

int
main()
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) >= 0)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        
        SDL_Window* Window = SDL_CreateWindow("SelfmadeX",
                                              SDL_WINDOWPOS_UNDEFINED,
                                              SDL_WINDOWPOS_UNDEFINED,
                                              WIDTH, HEIGHT,
                                              SDL_WINDOW_OPENGL);
        if (Window)
        {
            SDL_GLContext WindowContext = SDL_GL_CreateContext(Window);
            if (VSYNC(-1) < 0) VSYNC(1);
            if (WindowContext)
            {
                gladLoadGLLoader(SDL_GL_GetProcAddress);
                glViewport(0, 0, WIDTH, HEIGHT);
                glClearColor(0.15f, 0.3f, 0.3f, 1);
                
                bool Running = true;

                GLuint Vshader = ReadAndCompileShader("shaders/vertex.glsl", GL_VERTEX_SHADER);
                GLuint Fshader = ReadAndCompileShader("shaders/fragment.glsl", GL_FRAGMENT_SHADER);

                GLuint Program = glCreateProgram();
                glAttachShader(Program, Vshader);
                glAttachShader(Program, Fshader);
                glLinkProgram(Program);
                    
                LogProgramLinking(Program);
                glUseProgram(Program);
                glDeleteShader(Vshader);
                glDeleteShader(Fshader);
                

                GLfloat vertexData[] =
                {
                    -0.5f, -0.5f,
                     0.5f, -0.5f,
                     0.0f,  0.5f
                };

                GLuint VBO, VAO;
                glGenVertexArrays(1, &VAO);
                glBindVertexArray(VAO);
                
                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), BUFFER_OFFSET(0));
                glEnableVertexAttribArray(0);

                Uint32 currentTime = 0;
                Uint32 lastFrameTime = SDL_GetTicks();
                while (Running)
                {
                    SDL_Event e;
                    if (SDL_PollEvent(&e))
                    {
                        switch (e.type)
                        {
                            case SDL_QUIT:
                                Running = false;
                                break;
                            default: break;
                        }
                    }

                    glClear(GL_COLOR_BUFFER_BIT);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                    SDL_GL_SwapWindow(Window);

                    currentTime = SDL_GetTicks();
                    //SDL_Log("frame time: %u", currentTime - lastFrameTime);
                    lastFrameTime = currentTime;
                }
            }
            else
            {
                // TODO(dave): Logging
            }
        }
        else
        {
            // TODO(dave): Logging
        }
    }
    else
    {
        // TODO(dave): Logging
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    }

    return 0;
}
