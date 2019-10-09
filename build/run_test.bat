@echo off
call build tests quietly
tests.exe %* > logs/_tests.log