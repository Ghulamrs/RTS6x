@echo off
rem Spec: none - the Windows half of tools/referee: every <name>.out shipped beside this file runs on
rem the CCS 5.5 C6747 cycle-accurate simulator through par-one.cmd, MAXRUNS at once; a session that
rem never started is run again alone. The results and the CIO output go to out\ for the Mac to judge.
setlocal enabledelayedexpansion
set W=%~dp0
set W=%W:~0,-1%
if "%MAXRUNS%"=="" set MAXRUNS=10
cd /d "%W%"
if exist out rmdir /s /q out
mkdir out
if exist ws rmdir /s /q ws 2>nul
mkdir ws 2>nul
set WANT=0
for %%p in (*.out) do set /a WANT+=1
for %%p in (*.out) do call :start %%~np
:gather
set DONE=0
for %%r in (*.result) do set /a DONE+=1
if !DONE! lss !WANT! (ping -n 11 127.0.0.1 >nul & goto gather)
for %%r in (*.result) do findstr /c:"count=" %%r >nul || (del %%r & call "%W%\par-one.cmd" %%~nr)
copy /y *.result out\ >nul 2>&1
copy /y *.stdout out\ >nul 2>&1
echo done> out\referee.done
exit /b 0

:start
for /f %%n in ('tasklist /fi "imagename eq eclipsec.exe" ^| find /c "eclipsec"') do set RUNNING=%%n
if %RUNNING% geq %MAXRUNS% (ping -n 6 127.0.0.1 >nul & goto start)
start "%1" /b cmd /c ""%W%\par-one.cmd" %1"
ping -n 3 127.0.0.1 >nul
exit /b 0
