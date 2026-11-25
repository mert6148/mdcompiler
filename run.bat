@echo off
setlocal

if not exist "build\mdcompiler.exe" (
    cmake -S "%~dp0" -B "%~dp0build" -G Ninja
    cmake --build "%~dp0build" --parallel
)

"%~dp0build\mdcompiler.exe" run "%~dp0examples\hello.cpp"
"%~dp0build\mdcompiler.exe" run "%~dp0examples\hello.c"
