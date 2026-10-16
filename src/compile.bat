@echo off
echo ========================================
echo Compiling C++20 Project...
echo ========================================

g++ main.cpp ui/*.cpp csm/*.cpp csm/components/*.cpp -o main -std=c++20

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build Failed!
    exit /b %errorlevel%
)

echo [SUCCESS] Build completed. Booting up...
echo ========================================
echo.
main.exe