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

#include "win32_selfmadex.h"
#include "common_selfmadex.h"
//#include "common_selfmadex.cpp"


global_variable bool32 GlobalRunning = true;

internal inline void*
Win32VirtualAlloc(SIZE_T size)
{
    LPVOID r = VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
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

internal inline bool
Win32VirtualFree(void *p)
{
    return VirtualFree(p, 0, MEM_RELEASE);
}

#if 0
// TODO(dave): Make this platform dependant and DON'T USE STREAMS!!
internal debug_file
Win32ReadFile(const char* filename)
{
    debug_file File = { };

    std::ifstream stream;
    stream.open(filename, std::ifstream::in | std::ifstream::ate);
    if (stream.is_open())
    {
        File.length = (Uint32)stream.tellg();
        if (File.length > 0)
        {
            stream.seekg(0);
            File.data = (GLchar *) Win32VirtualAlloc(File.length * sizeof(GLchar));
            stream.read(File.data, File.length);
            stream.close();
        }
        else ThrowErrorAndExit("Reading %s", filename);
    }
    else
    {
        // TODO(dave): Logging
        ThrowErrorAndExit("Opening %s", filename);
    }

    return File;
}
#endif

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

LRESULT CALLBACK
Win32MessagesCallback(HWND   Window,
                      UINT   Message,
                      WPARAM wParam,
                      LPARAM lParam)
{
    switch(Message)
    {
        case WM_DESTROY:
        case WM_CLOSE:
        {
            GlobalRunning = false;
        }
    }

    return DefWindowProc(Window, Message, wParam, lParam);
}

#if 1
int CALLBACK
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR     CommandLine,
    int       ShowCommand)
{
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MessagesCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "SelfmadeWindowClass";

    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            "SelfmadeX",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);

        if(Window)
        {
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    else
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }
            }
        }
        else
        {
            // TODO(dave): Logging
        }
    }

    return 0;
}
#else
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
                
                bool GlobalRunning = true;

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
                glDisable(GL_CULL_FACE);
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                mesh_data cubefloor = ImportOBJ("cubefloor.obj");
                
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

                    glClear(GL_COLOR_BUFFER_BIT);
                    Render(cubefloor);
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
#endif
