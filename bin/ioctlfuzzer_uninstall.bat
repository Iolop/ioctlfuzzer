@echo off

:: check for OS architecture
call .\_config.bat

%IOCTLFUZZER% --uninstall
