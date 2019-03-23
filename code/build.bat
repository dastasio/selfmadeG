@echo off

REM TODO - can we just build both with one exe?

REM -wd4201 -wd4189 -wd4456
set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHsc -Od -Oi -WX -W4 -wd4100 -DSELFX_UNICODE=1 -DSELFX_WIN32=1 -FAsc -Z7 -Fmwin32_selfmadex.map -I..\lib\include
set CommonLinkerFlags=-opt:ref ../lib/SDL2.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\code\win32_selfmadex.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
cl %CommonCompilerFlags% ..\code\win32_selfmadex.cpp /link %CommonLinkerFlags%
popd
