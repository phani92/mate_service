@echo off
title Mate Service Server
echo Starting Mate Service...
echo.

REM Check if Node is installed
where node >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: Node.js is not installed or not in your PATH.
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b
)

REM Install dependencies if node_modules is missing
if not exist "node_modules" (
    echo Installing dependencies...
    call npm install
)

echo.
echo Launching server...
echo Press Ctrl+C to stop the server.
echo.

node server.js

pause
