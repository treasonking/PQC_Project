@echo off
setlocal

if "%CMAKE_EXE%"=="" set "CMAKE_EXE=C:\Users\jho87\Downloads\cmake-4.3.1-windows-x86_64\cmake-4.3.1-windows-x86_64\bin\cmake.exe"
if "%BUILD_DIR%"=="" set "BUILD_DIR=build"

if not exist "%CMAKE_EXE%" (
  echo [ERROR] CMAKE_EXE not found: %CMAKE_EXE%
  echo Set CMAKE_EXE to your cmake.exe path.
  exit /b 1
)

if "%CC%"=="" (
  "%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -G "MinGW Makefiles"
) else (
  "%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_C_COMPILER="%CC%"
)
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" --build "%BUILD_DIR%"
if errorlevel 1 exit /b 1

echo [OK] Build complete
endlocal
