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

#define VSYNC(x) SDL_GL_SetSwapInterval(x)
#define WIDTH 1024
#define HEIGHT 720

#define global_variable static

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
                glClearColor(0, 0, 0, 1);
                
                bool Running = true;
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
                    SDL_GL_SwapWindow(Window);

                    currentTime = SDL_GetTicks();
                    SDL_Log("frame time: %u", currentTime - lastFrameTime);
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
