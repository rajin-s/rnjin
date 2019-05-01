@echo off

SET output_log=scons.log

if "%1%"=="" (goto make_default)
if %1% == clean (goto make_clean) else (goto make_arg)

echo "something went wrong :("
goto _exit

:make_clean
    cmd /c scons make=all -c > %output_log%
    goto _end

:make_arg
    cmd /c scons make=%1 > %output_log%
    goto _end

:make_default
    cmd /c scons make=all > %output_log%
    goto _end

:_end
    if "%2%"=="run" (goto run) else (goto _exit)

:run
    cmd /c .\rnjin.exe
    goto _exit

:_exit
