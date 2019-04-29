@echo off

SET output_log = scons.txt

if "%1%"=="" (goto make_default)
if %1% == clean (goto make_clean) else (goto make_arg)

echo "something went wrong :("
goto end

:make_clean
    scons make=all -c > %output_log
    goto end

:make_arg
    scons make=%1 > %output_log
    goto end

:make_default
    scons make=all > %output_log
    goto end

:end
    echo "done!"