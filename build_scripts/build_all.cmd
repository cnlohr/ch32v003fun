@echo off
setlocal

set TARGET=%1
if [%1]==[] set TARGET=build

cd ..\examples

for /d %%i in (*) do make --directory=%%i %TARGET%

cd ..\build_scripts
