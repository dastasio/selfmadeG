#if !defined(SDL_SELFMADE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */

void *Win32VirtualAlloc(SIZE_T size, LPVOID BaseAddress = 0);
bool Win32VirtualFree(void *p);
debug_file SDLReadEntireFile(char *Filename);

#define SDL_SELFMADE_H
#endif
