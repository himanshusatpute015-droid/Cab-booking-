@echo off
REM Build CabGo on Windows with MinGW g++
echo Building CabGo...
g++ -std=c++14 -Wall -O2 -Iinclude src\main.cpp src\core\*.cpp src\models\*.cpp src\services\*.cpp src\web\*.cpp -o cabgo.exe -lws2_32
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b 1
)
echo Build successful. Run:  cabgo.exe   then open http://localhost:8080
