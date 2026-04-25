# Troubleshooting

## `cmake` not found

Windows에서 PATH에 CMake가 없으면 절대 경로로 실행한다.

```powershell
& 'C:\path\to\cmake.exe' --version
```

## C compiler not found

에러 예시:

- `CMAKE_C_COMPILER not set`
- `nmake not found`

대응:

- MinGW/clang/MSVC 중 하나 설치
- CMake 생성기와 컴파일러를 명시

예시:

```powershell
& 'C:\path\to\cmake.exe' -S . -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER="C:\path\to\gcc.exe"
```

Windows 배치 스크립트 사용 시:

```powershell
set TOOLCHAIN_BIN=C:\path\to\w64devkit\bin
set CMAKE_GENERATOR=MinGW Makefiles
set CC=C:\path\to\gcc.exe
scripts\build_windows.bat
```

## `make` not recognized on Windows

- `TOOLCHAIN_BIN`이 `w64devkit\bin`을 가리키는지 확인
- 또는 PowerShell 세션 PATH에 직접 추가

```powershell
$env:Path='C:\path\to\w64devkit\bin;' + $env:Path
```

## Git commit fails with author identity unknown

```powershell
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

## Security logging policy

- 에러 메시지에 키/공유비밀 바이트를 포함하지 않는다.
- 파일 경로/상태코드 중심으로 로그를 남긴다.

## CLI failure examples

없는 공개키 파일로 encaps 실행:

```powershell
.\build\pqc_cli.exe encaps --alg mlkem-ref --pub does_not_exist.key --ct out.ct --ss out.ss
```

예상 결과:

```text
failed to read public key file
```

손상된 secret key/ciphertext로 decaps 실행:

```powershell
.\build\pqc_cli.exe decaps --alg mlkem-ref --sec tests\data\bad.sec --ct tests\data\bad.ct --ss recovered.ss
```

예상 결과:

```text
failed to read input files
```

지원하지 않는 알고리즘 지정:

```powershell
.\build\pqc_cli.exe info --alg unknown
```

예상 결과:

```text
unsupported --alg value: unknown
```
