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

## Git commit fails with author identity unknown

```powershell
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

## Security logging policy

- 에러 메시지에 키/공유비밀 바이트를 포함하지 않는다.
- 파일 경로/상태코드 중심으로 로그를 남긴다.
