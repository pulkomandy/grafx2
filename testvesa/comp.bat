@echo off
50
wmake
if errorlevel 1 goto END
if not exist testvesa.exe goto END
if %0==COMP testvesa %1
:END