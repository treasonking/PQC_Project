# PQC Crypto Module Lab

개인 학습 프로젝트로, C 기반의 PQC(KEM) 모듈 구조를 설계하고 CLI, 테스트, 벤치마크를 함께 구성하는 것을 목표로 합니다.

현재 단계는 **Phase 1 완료 + Phase 2 준비 구조 반영** 상태입니다.
실제 ML-KEM 연동 전에 공통 API/오류 처리/CLI 뼈대를 안정화하고, 알고리즘 종속 로직 분리 구조를 먼저 적용했습니다.

## Why

- PQC 알고리즘을 단순 호출이 아니라 모듈화 관점으로 다루기 위함
- C 라이브러리 API 설계 + 예외 처리 + 테스트 경험 확보
- 이후 ML-KEM 참조 구현 연결을 위한 구조 선행

## Features (Phase 1)

- 공통 API
  - `pqc_kem_keypair`
  - `pqc_kem_encaps`
  - `pqc_kem_decaps`
  - `pqc_get_algorithm_name`
  - `pqc_get_sizes`
  - `secure_memzero`
- CLI
  - `help`
  - `info`
  - `keygen`
  - `encaps`
  - `decaps`
  - `benchmark` (더미)
  - `--alg <dummy|mlkem-ref>` 옵션
- 테스트
  - 더미 KEM 라운드트립 검증
  - 잘못된 인자 처리 검증
  - 백엔드 전환/오류 처리 negative 테스트

## Directory

```text
pqc-crypto-module-lab/
  ├─ CMakeLists.txt
  ├─ README.md
  ├─ docs/
  ├─ include/
  │   └─ pqc_module.h
  ├─ src/
  │   ├─ pqc_kem.c
  │   ├─ pqc_kem_backend.h
  │   ├─ pqc_module.c
  │   ├─ pqc_utils.c
  │   └─ main.c
  ├─ tests/
  │   ├─ test_kem.c
  │   └─ test_negative.c
  ├─ bench/
  │   ├─ benchmark_kem.c
  │   └─ parse_results.py
  ├─ third_party/
  │   └─ README.md
  └─ scripts/
      ├─ build_linux.sh
      ├─ build_windows.bat
      └─ run_all.sh
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

Windows (CMake path 지정):

```powershell
set CMAKE_EXE=C:\path\to\cmake.exe
set CC=C:\path\to\gcc.exe
scripts\build_windows.bat
```

## Run

```bash
./build/pqc_cli --help
./build/pqc_cli info
./build/pqc_cli keygen --pub pub.key --sec sec.key
./build/pqc_cli encaps --pub pub.key --ct out.ct --ss out.ss
./build/pqc_cli decaps --sec sec.key --ct out.ct --ss recovered.ss
./build/pqc_cli benchmark --iterations 1000
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Security Note

- 데모 단계에서도 비밀키/공유비밀을 콘솔에 출력하지 않습니다.
- 오류 로그에는 민감 데이터를 포함하지 않습니다.
- 민감 버퍼는 `secure_memzero`로 정리합니다.

## Phase 2 Prep Notes

- `src/pqc_kem.c`: 알고리즘 종속 로직(현재 dummy + ref placeholder) 분리
- `src/pqc_module.c`: 공통 API 엔트리(backend 라우팅)
- `third_party/`: 외부 참조 구현 연결 위치 명시
- 향후 ML-DSA 추가 시 같은 패턴으로 `*_backend` 구조 확장 가능

## Limitations

- 현재 암호 연산은 테스트용 더미 구현입니다.
- 상용 보안 용도로 사용하면 안 됩니다.
- 다음 단계에서 ML-KEM 참조 구현을 `third_party` 경로로 연동할 예정입니다.
