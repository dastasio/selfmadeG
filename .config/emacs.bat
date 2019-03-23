@echo off
REM NOTE - setting this to enable variable modification in for loop
setlocal EnableDelayedExpansion

set SourceDir = ""
set str1=""
for /f "tokens=1,2 delims=>" %%G in ('subst') do (
  set str1=%%G
  set str2=%%H
  if not %str1:X=%==!str1! set SourceDir=!str2:~1!
)
cd /D !SourceDir!\code
"C:\Program Files (x86)\emacs\bin\runemacs.exe" -q -l x:\.config\.emacs -l x:\.config\glsl-mode.el

x: