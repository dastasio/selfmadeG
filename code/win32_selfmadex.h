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
    Uint32  length;
    GLchar *data;
};

internal inline void* Win32VirtualAlloc(SIZE_T size);
internal inline bool Win32VirtualFree(void *p);
internal debug_file Win32ReadFile(const char* filename);


#define WIN32_SELFMADEX_H
#endif
