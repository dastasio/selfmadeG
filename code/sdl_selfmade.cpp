/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

#define SDL_MAIN_HANDLED
#include "glad.c"
#include "sdl2/sdl.h"

// NOTE(dave): WIN32_LEAN_AND_MEAN already defined by glad.c
#include <windows.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

#define VSYNC(x) SDL_GL_SetSwapInterval(x)
#define VERTEX_BUFFER 0
#define INDEX_BUFFER  1
#define WIDTH 1024
#define HEIGHT 720

#include "selfmade.h"
#include "sdl_selfmade.h"
#include "selfmade.cpp"

global_variable bool32 GlobalRunning = true;

inline void*
Win32VirtualAlloc(SIZE_T size, LPVOID BaseAddress)
{
    LPVOID r = VirtualAlloc(BaseAddress, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
#if SELFX_DEBUG
    if (!r)
    {
        char *buf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
            0, GetLastError(), 0, (LPSTR) &buf, 0, 0);

        ThrowErrorAndExit("Allocating virtual memory:\n%s", buf);
    }
#endif
    return r;
}

inline bool
Win32VirtualFree(void *p)
{
    return VirtualFree(p, 0, MEM_RELEASE);
}

// TODO(dave): Make this platform dependant and DON'T USE STREAMS!!
SDL_PLATFORM_READ_ENTIRE_FILE(SDLReadEntireFile)
{
    // TODO(dave): Use SDL i/o methods
    debug_file File = { };

    std::ifstream stream;
    stream.open(Filename, std::ifstream::in | std::ifstream::ate);
    if (stream.is_open())
    {
        File.Length = (Uint32)stream.tellg();
        if (File.Length > 0)
        {
            stream.seekg(0);
            File.Data = (GLchar *) Win32VirtualAlloc(File.Length * sizeof(GLchar));
            stream.read(File.Data, File.Length);
            stream.close();
        }
        else ThrowErrorAndExit("Reading %s", Filename);
    }
    else
    {
        // TODO(dave): Logging
        ThrowErrorAndExit("Opening %s", Filename);
    }

    return File;
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
                vec4 Vector = { 3, 5, 8, 7 };

                memory_block MainMemory = {};
                MainMemory.StorageSize = Megabytes(32);
                MainMemory.Storage = Win32VirtualAlloc(MainMemory.StorageSize, (LPVOID)Terabytes(2));
                MainMemory.SDLPlatformReadEntireFile = SDLReadEntireFile;

                gladLoadGLLoader(SDL_GL_GetProcAddress);
                
                Uint32 currentTime = 0;
                Uint32 lastFrameTime = SDL_GetTicks();
                while (GlobalRunning)
                {
                    SDL_Event e;
                    if (SDL_PollEvent(&e))
                    {
                        switch (e.type)
                        {
                            case SDL_QUIT:
                                GlobalRunning = false;
                                break;
                            default: break;
                        }
                    }

                    Render(&MainMemory);
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
