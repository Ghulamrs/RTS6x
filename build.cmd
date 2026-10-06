@echo off
rem RTS6x on Windows: the Makefile's steps - ar6x built with cl, each folder's C++ sources through cpp11
rem and its .s through asm6x, the provenance check, then ar6x packs build\rts6x.lib.
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

rem One cpp11 and one asm6x per folder of src\, each taking the folder's files as a pattern - both
rem expand * themselves, cmd not doing it. cpp11 -c writes its objects where it runs, asm6x where -o says.
for %%t in ("%CPP11%") do set "CPP11=%%~ft"
for %%t in ("%ASM6X%") do set "ASM6X=%%~ft"
for %%t in ("%OBJDIR%") do set "OBJDIR=%%~ft"
set "CPP11_AS=%ASM6X%"
for /d %%d in (src\*) do (
    if not exist "%OBJDIR%\%%~nxd" mkdir "%OBJDIR%\%%~nxd"
    if exist "%%d\*.cpp" (
        pushd "%OBJDIR%\%%~nxd"
        "%CPP11%" -arch tms6747 -nologo -O2 "-I%CD%\src\internal" -c "%CD%\%%d\*.cpp" >nul || (popd & echo build.cmd: cpp11 refused something in %%d & exit /b 1)
        popd
    )
    if exist "%%d\*.s" "%ASM6X%" "%%d\*.s" -o "%OBJDIR%\%%~nxd" || exit /b 1
)
rem The library's members, one a line for ar6x's @list: eh-none\ is the unwinder's stand-in, printf6x.lib's alone.
if exist build\rts6x.objs del build\rts6x.objs
for /r src %%f in (*.cpp *.s) do (
    set "REL=%%~dpf"
    set "REL=!REL:%CD%\src\=!"
    if /i not "!REL!"=="eh-none\" if /i not "!REL!"=="internal\" echo %OBJDIR%\!REL!%%~nf.obj>>build\rts6x.objs
)

rem The provenance check and the tests are shell scripts; Git for Windows carries the shell.
set "SH=%ProgramFiles%\Git\bin\sh.exe"
if not exist "%SH%" (echo build.cmd: no sh at %SH% - the provenance check needs one & exit /b 1)
"%SH%" tools/provenance || exit /b 1
build\ar6x.exe -r build\rts6x.lib @build\rts6x.objs || exit /b 1
echo build.cmd: build\rts6x.lib
rem printf6x.lib: printf, fprintf, sprintf and wprintf - printf6x.members lists them, as for make.
set "POBJS="
for /f "usebackq delims=" %%m in ("printf6x.members") do (
    set "M=%%m"
    set "M=!M:src/=!"
    set "M=!M:/=\!"
    set "M=!M:.cpp=.obj!"
    set "M=!M:.s=.obj!"
    set "POBJS=!POBJS! "%OBJDIR%\!M!""
)
build\ar6x.exe -r build\printf6x.lib %POBJS% || exit /b 1
echo build.cmd: build\printf6x.lib

if /i not "%~1"=="check" exit /b 0
if "%C90%"=="" set "C90=%RIDEBIN%\c90.exe"
if "%LNK6X%"=="" set "LNK6X=%RIDEBIN%\lnk6x.exe"
if "%VM%"=="" set "VM=%RIDEBIN%\vm6747.exe"
if "%VMSIM%"=="" set "VMSIM=%RIDEBIN%\vm6747sim.exe"
"%SH%" tests/run.sh
