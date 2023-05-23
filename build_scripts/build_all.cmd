@echo off
setlocal

set TARGET=%1
if [%1]==[] set TARGET=build

for /d %%i in (*) do make --directory=%%i %TARGET%
