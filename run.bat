@echo off
setlocal EnableExtensions

set "ROOT=%~dp0."
set "BUILD_DIR=%ROOT%\build\runner"

where cmake >nul 2>&1
if errorlevel 1 goto :missing_cmake

echo Configuring mdcompiler...
cmake -S "%ROOT%" -B "%BUILD_DIR%"
if errorlevel 1 goto :build_failed

echo Building mdcompiler...
cmake --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 goto :build_failed

if exist "%BUILD_DIR%\Release\mdcompiler.exe" (
    set "COMPILER=%BUILD_DIR%\Release\mdcompiler.exe"
) else if exist "%BUILD_DIR%\mdcompiler.exe" (
    set "COMPILER=%BUILD_DIR%\mdcompiler.exe"
)

if not defined COMPILER goto :compiler_missing

if exist "%ProgramFiles%\LLVM\bin\clang.exe" (
    set "PATH=%ProgramFiles%\LLVM\bin;%PATH%"
)

where clang >nul 2>&1
if not errorlevel 1 goto :run_compiler
where gcc >nul 2>&1
if not errorlevel 1 goto :run_compiler
where cl >nul 2>&1
if not errorlevel 1 goto :run_compiler
echo Error: No C compiler was found on PATH. Install LLVM or use a Visual Studio Developer Command Prompt.
exit /b 1

:run_compiler
if "%~1"=="" (
    "%COMPILER%" --help
) else (
    "%COMPILER%" %*
)
exit /b %ERRORLEVEL%

:missing_cmake
echo Error: CMake was not found on PATH. Install CMake and restart the terminal.
exit /b 1

:build_failed
echo Error: mdcompiler could not be built.
exit /b 1

:compiler_missing
echo Error: The build succeeded but mdcompiler.exe was not found.
exit /b 1

run.bat --help
run.bat build "%~dp0examples\hello.cpp"
run.bat run "%~dp0examples\hello.c"
