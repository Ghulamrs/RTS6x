@echo off
rem Spec: none - the Windows half of tools/referee: every <name>.out shipped beside this file runs on
rem the CCS 5.5 C6747 cycle-accurate simulator through par-one.cmd, MAXRUNS at once. The results and the
rem CIO output go to out\ for the Mac to judge; a run always finishes, an image with no answer says so.
setlocal enabledelayedexpansion
if /i "%~1"=="one" goto :one
set W=%~dp0
set W=%W:~0,-1%
if "%MAXRUNS%"=="" set MAXRUNS=10
rem REFTIMEOUT minutes from an image's first start: then it is stopped, marked did-not-run, and passed.
if "%REFTIMEOUT%"=="" set REFTIMEOUT=35
set /a LIMIT=REFTIMEOUT*60
rem REFSTART seconds for a session to reach its program: a CIO log by then, or the start was lost.
if "%REFSTART%"=="" set REFSTART=180
cd /d "%W%"
if exist out rmdir /s /q out
mkdir out
rem The host takes a path from the root, /tmp/x, as one under the program's own folder: tmp\x here.
mkdir tmp 2>nul
if exist ws rmdir /s /q ws 2>nul
mkdir ws 2>nul
del /q *.result *.stdout *.fin *.dead *.t0 *.ta *.tries 2>nul
set WANT=0
for %%p in (*.out) do (set /a WANT+=1 & set IMG!WANT!=%%~np)
set STARTED=0
:loop
call :now
set DONE=0
for %%f in (*.fin) do set /a DONE+=1
if !DONE! geq !WANT! goto :finish
set /a RUNNING=STARTED-DONE
if !RUNNING! lss !MAXRUNS! if !STARTED! lss !WANT! (
    set /a STARTED+=1
    for %%i in (!STARTED!) do set N=!IMG%%i!
    echo !NOW!> "!N!.t0"
    start "!N!" /b cmd /c ""%~f0" one !N!"
    ping -n 3 127.0.0.1 >nul
    goto :loop
)
for %%t in (*.t0) do if not exist "%%~nt.fin" (
    set /p T0=< "%%t"
    set /a "AGE=(NOW-T0+86400)%%86400"
    if !AGE! gtr !LIMIT! (call :giveup %%~nt !AGE!) else if exist "%%~nt.ta" if not exist "%%~nt.stdout" (
        set /p TA=< "%%~nt.ta"
        set /a "AGE=(NOW-TA+86400)%%86400"
        if !AGE! gtr !REFSTART! (del "%%~nt.ta" & call :stop %%~nt)
    )
)
ping -n 6 127.0.0.1 >nul
goto :loop

:finish
for %%r in (*.result) do findstr /c:"count=" "%%r" >nul || (echo referee: %%~nr did not run & type "%%r")
copy /y *.result out\ >nul 2>&1
copy /y *.stdout out\ >nul 2>&1
copy /y *.tries out\ >nul 2>&1
echo done> out\referee.done
exit /b 0

rem The second of the day from %TIME%, HH:MM:SS; an age is taken modulo a day, so a midnight costs nothing.
:now
for /f "tokens=1-3 delims=:.," %%a in ("%TIME: =0%") do set /a NOW=(1%%a-100)*3600+(1%%b-100)*60+(1%%c-100)
exit /b 0

rem An image past its time: its simulator stopped, and marked did-not-run.
:giveup
echo.> "%1.dead"
call :stop %1
(echo BOX %1 did-not-run timeout_s=%2)> "%1.result"
echo.> "%1.fin"
exit /b 0

rem One image's own simulator stopped, found by the workspace par-one gave it.
:stop
powershell -NoProfile -Command "$n=[regex]::Escape('%1'); Get-CimInstance Win32_Process | Where-Object { $_.Name -eq 'eclipsec.exe' -and $_.CommandLine -match ('\\ws\\' + $n + '\x22') } | ForEach-Object { taskkill /T /F /PID $_.ProcessId }" >nul 2>&1
exit /b 0

rem One image, as "referee.cmd one <name>": a start the simulator lost - no count in the result, or no
rem CIO log after REFSTART seconds - is tried twice more after a pause, unless the image was given up.
:one
cd /d "%~dp0"
set N=%~2
set TRY=0
:again
set /a TRY+=1
del "%N%.stdout" "%N%.result" 2>nul
call :now
echo !NOW!> "%N%.ta"
rem A cmd of its own: par-one.cmd stops at a syntax error when the log holds no RESULT line.
cmd /c ""%~dp0par-one.cmd" %N%"
del "%N%.ta" 2>nul
if not exist "%N%.result" (echo BOX %N% no-result)> "%N%.result"
findstr /c:"count=" "%N%.result" >nul 2>&1 && goto :onedone
if exist "%N%.dead" goto :onedone
(echo try !TRY!: no count & type "%N%.result" & findstr /c:"SEVERE" /c:"Can't" "%N%.log")>> "%N%.tries" 2>nul
if !TRY! lss 3 (
    set /a WAIT=TRY*10+1
    ping -n !WAIT! 127.0.0.1 >nul
    if not exist "%N%.dead" goto :again
)
:onedone
if not exist "%N%.dead" echo.> "%N%.fin"
exit /b 0
