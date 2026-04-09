@echo off
setlocal

if "%CMAKE_EXE%"=="" set "CMAKE_EXE=C:\Users\jho87\Downloads\cmake-4.3.1-windows-x86_64\cmake-4.3.1-windows-x86_64\bin\cmake.exe"
if "%BUILD_DIR%"=="" set "BUILD_DIR=build"
if "%CMAKE_GENERATOR%"=="" set "CMAKE_GENERATOR=MinGW Makefiles"
if "%TOOLCHAIN_BIN%"=="" set "TOOLCHAIN_BIN=C:\Users\jho87\Downloads\w64devkit\bin"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

if exist "%TOOLCHAIN_BIN%" (
  set "PATH=%TOOLCHAIN_BIN%;%PATH%"
)

if not exist "%CMAKE_EXE%" (
  echo [ERROR] CMAKE_EXE not found: %CMAKE_EXE%
  echo Set CMAKE_EXE to your cmake.exe path.
  exit /b 1
)

if /I "%CLEAN_BUILD%"=="1" (
  if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
  )
)

if "%CC%"=="" (
  "%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE="%BUILD_TYPE%"
) else (
  "%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -G "%CMAKE_GENERATOR%" -DCMAKE_C_COMPILER="%CC%" -DCMAKE_BUILD_TYPE="%BUILD_TYPE%"
)
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" --build "%BUILD_DIR%"
if errorlevel 1 exit /b 1

echo [OK] Build complete
endlocal
