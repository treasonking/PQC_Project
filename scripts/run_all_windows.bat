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

"%BUILD_DIR%\pqc_cli.exe" info --alg mlkem-ref --sig-alg mldsa-ref
if errorlevel 1 exit /b 1

"%BUILD_DIR%\pqc_cli.exe" benchmark --alg mlkem-ref --sig-alg mldsa-ref --iterations 100 --out bench_result.csv
if errorlevel 1 exit /b 1

set "PARSE_OK=0"
where py >nul 2>nul
if %errorlevel%==0 (
  py -3 bench\parse_results.py bench_result.csv > bench_summary.txt 2>nul
  if not errorlevel 1 (
    set "PARSE_OK=1"
  )
)

if "%PARSE_OK%"=="0" (
  where python >nul 2>nul
  if %errorlevel%==0 (
    python bench\parse_results.py bench_result.csv > bench_summary.txt 2>nul
    if not errorlevel 1 (
      set "PARSE_OK=1"
    )
  )
)

if "%PARSE_OK%"=="1" (
  type bench_summary.txt
) else (
  echo [WARN] python runtime not available; skip benchmark summary parse
)

echo [OK] run_all_windows complete
endlocal
