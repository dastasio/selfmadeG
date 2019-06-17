@echo off
REM NOTE - setting this to enable variable modification in for loop
REM setlocal EnableDelayedExpansion

REM TODO - can we just build both with one exe?

REM -wd4201 -wd4189 -wd4456
set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHsc -Od -Oi -WX -W4 -wd4100 -wd4996 -DSELFX_DEBUG=1 -DSELFX_INTERNAL=1 -DSELFX_SLOW=1 -DSELFX_WIN32=1 -FAsc -Z7 -Fmwin32_selfmadex.map -I..\lib\include
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

REM IF NOT EXIST shaders (
REM  set SourceDir = ""
REM  set str1=""
REM  for /f "tokens=1,2 delims=>" %%G in ('subst') do (
REM    set str1=%%G
REM    set str2=%%H
REM    if not %str1:X=%==!str1! set SourceDir=!str2!
REM  )
REM
REM  mklink /D "!SourceDir!\build\shaders" "!SourceDir!\code\shaders"
REM )

REM 32-bit build
REM cl %CommonCompilerFlags% ..\code\win32_selfmadex.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
cl %CommonCompilerFlags% ..\code\win32_selfmadex.cpp -Fmwin32_selfmadex.map /link %CommonLinkerFlags%

popd
