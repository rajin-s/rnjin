@echo off
call build rnjin quietly
rnjin.exe %* > logs/_run.log