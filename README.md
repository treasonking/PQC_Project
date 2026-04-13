# PQC Crypto Module Lab

개인 학습 프로젝트로, C 기반의 PQC(KEM) 모듈 구조를 설계하고 CLI, 테스트, 벤치마크를 함께 구성하는 것을 목표로 합니다.

현재 단계는 **Phase 1 완료 + Phase 2(ML-KEM 참조 구현 연결) 진행** 상태입니다.
공통 API/오류 처리/CLI 뼈대를 유지한 채, `mlkem-ref` 백엔드를 PQClean 기반 구현으로 연결했습니다.

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
  - `sig-keygen`
  - `encaps`
  - `decaps`
  - `sign`
  - `verify`
  - `benchmark` (더미)
  - `--alg <dummy|mlkem-ref>` 옵션
  - `--sig-alg <dummy-dsa|mldsa-ref>` 옵션
- 테스트
  - 더미 KEM 라운드트립 검증
  - 참조 ML-KEM 라운드트립 검증
  - 100회 반복 실행 검증
  - 잘못된 인자 처리 검증
  - 백엔드 전환/오류 처리 negative 테스트
  - CLI 입력 파일 누락/손상 실패 테스트

## Benchmark Results (Example)

아래 값은 로컬 Windows + w64devkit 환경에서 `mlkem-ref`로 측정한 예시다.

| Algorithm | Iterations | Keygen (ms) | Encaps (ms) | Decaps (ms) |
| --- | ---: | ---: | ---: | ---: |
| ML-KEM-768-PQCLEAN | 100 | 0.540 | 0.500 | 0.080 |

주의:
- 측정값은 실행 환경과 부하에 따라 달라진다.
- 절대 성능 수치보다, 같은 환경에서의 상대 비교 지표로 사용한다.

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
  │   ├─ test_negative.c
  │   ├─ test_ref_kem.c
  │   └─ test_sig.c
  ├─ bench/
  │   ├─ benchmark_kem.c
  │   └─ parse_results.py
  ├─ third_party/
  │   ├─ mlkem_pqclean/
  │   ├─ mldsa_pqclean/
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
set TOOLCHAIN_BIN=C:\path\to\w64devkit\bin
scripts\build_windows.bat
```

Windows 전체 실행(빌드+테스트+벤치):

```powershell
scripts\run_all_windows.bat
```

## Run

```bash
./build/pqc_cli --help
./build/pqc_cli info
./build/pqc_cli keygen --pub pub.key --sec sec.key
./build/pqc_cli sig-keygen --pub sig_pub.key --sec sig_sec.key
./build/pqc_cli sig-keygen --sig-alg mldsa-ref --pub sig_pub.key --sec sig_sec.key
./build/pqc_cli encaps --pub pub.key --ct out.ct --ss out.ss
./build/pqc_cli decaps --sec sec.key --ct out.ct --ss recovered.ss
./build/pqc_cli sign --sec sig_sec.key --msg message.txt --sig message.sig
./build/pqc_cli verify --pub sig_pub.key --msg message.txt --sig message.sig
./build/pqc_cli benchmark --iterations 1000
./build/pqc_cli benchmark --alg mlkem-ref --sig-alg mldsa-ref --iterations 1000 --out bench_result.csv
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## CI

GitHub Actions CI는 `main` push와 PR에서 아래를 자동 수행한다.

- Ubuntu: configure/build/test + mlkem-ref smoke run
- Windows(MSYS2 MinGW): configure/build/test + mlkem-ref smoke run

워크플로 파일:
- `.github/workflows/ci.yml`

## Security Note

- 데모 단계에서도 비밀키/공유비밀을 콘솔에 출력하지 않습니다.
- 오류 로그에는 민감 데이터를 포함하지 않습니다.
- 민감 버퍼는 `secure_memzero`로 정리합니다.

## What I Learned

- 암호 모듈에서는 알고리즘 정확성만큼 API 일관성과 실패 동작이 중요하다.
- 민감 데이터가 로그나 예외 경로로 새지 않도록 초기 설계부터 규칙을 넣어야 한다.
- 기능 구현 이후 테스트/벤치/문서를 붙여야 실제 개발 역량으로 설명 가능해진다.

## Phase 2 Notes

- `src/pqc_kem.c`: 알고리즘 종속 로직(현재 dummy + PQClean ML-KEM-768) 분리
- `src/pqc_module.c`: 공통 API 엔트리(backend 라우팅)
- `third_party/mlkem_pqclean`: 참조 구현 vendor 파일
- 향후 ML-DSA 추가 시 같은 패턴으로 `*_backend` 구조 확장 가능

## Limitations

- 현재 암호 연산은 테스트용 더미 구현입니다.
- `mlkem-ref`는 참조 구현 기반이며 API/테스트 목적의 통합 단계입니다.
- 상용 보안 용도로 사용하면 안 됩니다.

## Future Work

- ML-DSA(서명) 백엔드 추가 및 KEM/Signature 통합 CLI 제공
- CI(GitHub Actions)에서 빌드/테스트/정적 점검 자동화
- Linux 실측 벤치와 Windows 실측 벤치를 같은 조건으로 비교 표준화
- KCMVP 관점의 추가 요구사항(키 관리, 인증 경계, 운영 절차) 문서화
