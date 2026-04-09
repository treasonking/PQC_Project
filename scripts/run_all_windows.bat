@echo off
setlocal

if "%CMAKE_EXE%"=="" set "CMAKE_EXE=C:\Users\jho87\Downloads\cmake-4.3.1-windows-x86_64\cmake-4.3.1-windows-x86_64\bin\cmake.exe"
if "%CTEST_EXE%"=="" set "CTEST_EXE=C:\Users\jho87\Downloads\cmake-4.3.1-windows-x86_64\cmake-4.3.1-windows-x86_64\bin\ctest.exe"
if "%BUILD_DIR%"=="" set "BUILD_DIR=build"
if "%TOOLCHAIN_BIN%"=="" set "TOOLCHAIN_BIN=C:\Users\jho87\Downloads\w64devkit\bin"
if exist "%TOOLCHAIN_BIN%" set "PATH=%TOOLCHAIN_BIN%;%PATH%"

call "%~dp0build_windows.bat"
if errorlevel 1 exit /b 1

"%CTEST_EXE%" --test-dir "%BUILD_DIR%" --output-on-failure
if errorlevel 1 exit /b 1

"%BUILD_DIR%\pqc_cli.exe" info --alg mlkem-ref
if errorlevel 1 exit /b 1

"%BUILD_DIR%\pqc_cli.exe" benchmark --alg mlkem-ref --iterations 100 --out bench_result.csv
if errorlevel 1 exit /b 1

echo [OK] run_all_windows complete
endlocal
