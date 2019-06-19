#if !defined(WIN32_SELFMADEX_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

struct debug_file
{
    uint32  length;
    GLchar *data;
};

void *Win32VirtualAlloc(SIZE_T size, LPVOID BaseAddress = 0);
bool Win32VirtualFree(void *p);
debug_file SDLReadEntireFile(char *Filename);

#define WIN32_SELFMADEX_H
#endif
