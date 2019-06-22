@echo off

echo REPLACED BY `build.py`

REM SET output_log=scons.log

REM if "%1%"=="" (goto make_default)
REM if %1% == clean (goto make_clean) else (goto make_arg)

REM echo "something went wrong :("
REM goto _exit

REM :make_clean
REM     cmd /c scons make=all -c > %output_log%
REM     goto _end

REM :make_arg
REM     cmd /c scons make=%1 > %output_log%
REM     set /p build_result=<%output_log%
REM     echo %build_result%
REM     goto _end

REM :make_default
REM     cmd /c scons make=all > %output_log%
REM     goto _end

REM :_end
REM     if "%2%"=="run" (goto run) else (goto _exit)

REM :run
REM     cmd /c .\rnjin.exe
REM     goto _exit

REM :_exit