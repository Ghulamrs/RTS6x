@echo off
rem RTS6x on Windows: the Makefile's steps - ar6x built with cl, every C++ source through cpp11 and
rem asm6x, every .s through asm6x, the provenance check, then ar6x packs build\rts6x.lib.
rem The tools: CPP11, ASM6X named, else beside this checkout, else an installed RIDE's bin.
rem   build.cmd           the library        build.cmd check     the library, then tests\run.sh
setlocal enabledelayedexpansion
cd /d "%~dp0"
set "RIDEBIN=%ProgramFiles%\RIDE 5.0\bin"
if "%CPP11%"=="" if exist "..\C++Optimize\cpp11.exe" set "CPP11=..\C++Optimize\cpp11.exe"
if "%CPP11%"=="" set "CPP11=%RIDEBIN%\cpp11.exe"
if "%ASM6X%"=="" if exist "..\ASM6x\build\asm6x.exe" set "ASM6X=..\ASM6x\build\asm6x.exe"
if "%ASM6X%"=="" set "ASM6X=%RIDEBIN%\asm6x.exe"
if not exist "%CPP11%" (echo build.cmd: no cpp11 at %CPP11% & exit /b 1)
if not exist "%ASM6X%" (echo build.cmd: no asm6x at %ASM6X% & exit /b 1)
set "OBJDIR=..\build\RTS6x\obj"
if not exist build mkdir build
if not exist "%OBJDIR%" mkdir "%OBJDIR%"

rem Visual Studio through a file, as RIDE's build.bat does: for /f cannot run a quoted path.
if not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath > "%TEMP%\rts6x-vs.txt"
    set "VSPATH="
    set /p VSPATH=<"%TEMP%\rts6x-vs.txt"
    del "%TEMP%\rts6x-vs.txt"
    if "!VSPATH!"=="" (echo build.cmd: no Visual Studio & exit /b 1)
    call "!VSPATH!\VC\Auxiliary\Build\vcvars64.bat" >nul
)
cl /nologo /std:c++14 /O2 /W4 /WX /EHsc /D_CRT_SECURE_NO_WARNINGS /Fo"%OBJDIR%\\" /Febuild\ar6x.exe tools\ar6x\ar6x.cpp >nul || (echo build.cmd: ar6x did not build & exit /b 1)

set "OBJS="
for /r src %%f in (*.cpp) do (
    set "REL=%%~dpf"
    set "REL=!REL:%CD%\src\=!"
    if not exist "%OBJDIR%\!REL!" mkdir "%OBJDIR%\!REL!"
    "%CPP11%" -arch tms6747 -nologo -O2 -Isrc\internal -S "%%f" -o "%OBJDIR%\!REL!%%~nf.s" >nul || (echo build.cmd: cpp11 refused %%f & exit /b 1)
    "%ASM6X%" "%OBJDIR%\!REL!%%~nf.s" -o "%OBJDIR%\!REL!%%~nf.obj" || exit /b 1
    set "OBJS=!OBJS! "%OBJDIR%\!REL!%%~nf.obj""
)
for /r src %%f in (*.s) do (
    set "REL=%%~dpf"
    set "REL=!REL:%CD%\src\=!"
    if not exist "%OBJDIR%\!REL!" mkdir "%OBJDIR%\!REL!"
    "%ASM6X%" "%%f" -o "%OBJDIR%\!REL!%%~nf.obj" || exit /b 1
    set "OBJS=!OBJS! "%OBJDIR%\!REL!%%~nf.obj""
)

rem The provenance check and the tests are shell scripts; Git for Windows carries the shell.
set "SH=%ProgramFiles%\Git\bin\sh.exe"
if not exist "%SH%" (echo build.cmd: no sh at %SH% - the provenance check needs one & exit /b 1)
"%SH%" tools/provenance || exit /b 1
build\ar6x.exe -r build\rts6x.lib %OBJS% || exit /b 1
echo build.cmd: build\rts6x.lib

if /i not "%~1"=="check" exit /b 0
if "%C90%"=="" set "C90=%RIDEBIN%\c90.exe"
if "%LNK6X%"=="" set "LNK6X=%RIDEBIN%\lnk6x.exe"
if "%VM%"=="" set "VM=%RIDEBIN%\vm6747.exe"
if "%VMSIM%"=="" set "VMSIM=%RIDEBIN%\vm6747sim.exe"
"%SH%" tests/run.sh
