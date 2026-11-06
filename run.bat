@echo off
if not exist cabgo.exe call build.bat
if %errorlevel% neq 0 exit /b 1
start "" http://localhost:8080
cabgo.exe 8080
