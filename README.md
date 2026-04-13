# PQC Crypto Module Lab

C 기반 PQC 모듈 개인 프로젝트입니다.  
목표는 **KEM + Signature를 공통 API/CLI/테스트/벤치마크/문서화까지 연결**해서 실제 개발 역량을 보여주는 것입니다.

## Current Scope

### KEM
- API: `pqc_kem_keypair`, `pqc_kem_encaps`, `pqc_kem_decaps`
- CLI: `keygen`, `encaps`, `decaps`
- 알고리즘 선택: `--alg <dummy|mlkem-ref>`

### Signature
- API: `pqc_sig_keypair`, `pqc_sig_sign`, `pqc_sig_verify`
- CLI: `sig-keygen`, `sign`, `verify`
- 알고리즘 선택: `--sig-alg <dummy-dsa|mldsa-ref>`

### Benchmark
- **공식 벤치 엔트리포인트는 `pqc_cli benchmark` 하나로 통일**되어 있습니다.
- `bench/parse_results.py`는 벤치 결과(TXT/CSV) 요약 파서입니다.

## Implementation Status

| Area | Dummy | PQClean Ref | Notes |
| --- | --- | --- | --- |
| ML-KEM-768 | Yes | Yes (`mlkem-ref`) | KEM keygen/encaps/decaps 구현 |
| ML-DSA-65 | Yes | Yes (`mldsa-ref`) | Signature keygen/sign/verify 구현 |
| CLI | Yes | Yes | KEM/Signature/Benchmark 통합 |
| Tests | Yes | Yes | 단위/음성/KAT 포함 |
| CI | Yes | Yes | Linux + Windows(MSYS2 MinGW) |

## Build Matrix (CMake Conditional)

| Condition | KEM Backend | Signature Backend |
| --- | --- | --- |
| `third_party/mlkem_pqclean` + `common` 존재, `third_party/mldsa_pqclean` 존재 | Dummy + ML-KEM Ref | Dummy + ML-DSA Ref |
| ML-KEM 경로 미존재 | Dummy only | (ML-DSA 조건에 따름) |
| ML-DSA 경로 미존재 | (ML-KEM 조건에 따름) | Dummy only |

참고:
- KEM Ref 활성 플래그: `PQC_ENABLE_MLKEM_REF`
- Signature Ref 활성 플래그: `PQC_ENABLE_MLDSA_REF`

## Directory

```text
pqc-crypto-module-lab/
  ├─ CMakeLists.txt
  ├─ README.md
  ├─ docs/
  │   ├─ architecture.md
  │   ├─ benchmark.md
  │   ├─ security.md
  │   ├─ troubleshooting.md
  │   └─ portfolio-summary.md
  ├─ include/
  │   └─ pqc_module.h
  ├─ src/
  │   ├─ main.c
  │   ├─ pqc_module.c
  │   ├─ pqc_kem.c
  │   ├─ pqc_sig.c
  │   ├─ pqc_kem_backend.h
  │   ├─ pqc_sig_backend.h
  │   └─ pqc_utils.c
  ├─ tests/
  │   ├─ test_kem.c
  │   ├─ test_ref_kem.c
  │   ├─ test_sig.c
  │   ├─ test_negative.c
  │   ├─ test_kat_mldsa.c
  │   └─ data/
  │       ├─ bad.ct
  │       ├─ bad.sec
  │       ├─ kat_mldsa_msg.txt
  │       ├─ kat_mldsa_pub.key
  │       ├─ kat_mldsa_sec.key
  │       └─ kat_mldsa.sig
  ├─ bench/
  │   └─ parse_results.py
  ├─ third_party/
  │   ├─ mlkem_pqclean/
  │   ├─ mldsa_pqclean/
  │   └─ README.md
  ├─ scripts/
  │   ├─ build_linux.sh
  │   ├─ build_windows.bat
  │   ├─ run_all.sh
  │   └─ run_all_windows.bat
  └─ .github/workflows/
      ├─ ci.yml
      └─ quality.yml
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

Windows 예시:

```powershell
set CMAKE_EXE=C:\path\to\cmake.exe
set CC=C:\path\to\gcc.exe
set TOOLCHAIN_BIN=C:\path\to\w64devkit\bin
scripts\build_windows.bat
```

## Run

```bash
./build/pqc_cli info --alg mlkem-ref --sig-alg mldsa-ref
./build/pqc_cli keygen --alg mlkem-ref --pub kem_pub.key --sec kem_sec.key
./build/pqc_cli sig-keygen --sig-alg mldsa-ref --pub sig_pub.key --sec sig_sec.key
./build/pqc_cli sign --sig-alg mldsa-ref --sec sig_sec.key --msg message.txt --sig message.sig
./build/pqc_cli verify --sig-alg mldsa-ref --pub sig_pub.key --msg message.txt --sig message.sig
./build/pqc_cli benchmark --alg mlkem-ref --sig-alg mldsa-ref --iterations 1000 --out bench_result.csv
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

테스트 범위 요약:
- KEM roundtrip/repeat/tamper
- Signature roundtrip/tamper/empty message/length error
- CLI 파일 오류 케이스
- ML-DSA KAT(저장된 벡터 verify)

## Test Evidence (Current)

| Test | Purpose |
| --- | --- |
| `test_kem` | KEM 정상/반복/변조 ciphertext 실패 |
| `test_ref_kem` | ML-KEM 참조 백엔드 roundtrip |
| `test_sig` | 서명 정상/변조/길이 오류/빈 메시지 |
| `test_kat_mldsa` | 저장된 ML-DSA 벡터 verify (KAT 성격) |
| `test_negative` | 알고리즘/입력 오류 처리 |
| `cli_missing_input_file` | CLI 입력 파일 누락 실패 |
| `cli_corrupted_key_file` | CLI 손상 키 파일 실패 |

## Verification Notes

- 프로젝트는 PQClean 참조 구현을 통합해 동작합니다.
- 다만 KAT는 현재 **ML-DSA verify 기반 벡터 검증** 중심이며, 표준 벡터 전체를 자동화한 상태는 아닙니다.
- 향후 개선: NIST/PQClean 공식 벡터 기반 자동 KAT 확장.

## Security Notes (Honest)

- 민감 데이터(비밀키/공유비밀/서명 내부값) 원문을 로그에 출력하지 않습니다.
- 민감 버퍼는 `secure_memzero`로 정리합니다.
- 더미 알고리즘은 `rand()` 기반으로 암호학적으로 안전하지 않습니다.
- constant-time/side-channel 완전 대응은 현재 범위 밖입니다.
- 이 저장소는 학습/포트폴리오 목적이며 상용 배포 수준의 보증을 제공하지 않습니다.

## CI

GitHub Actions (`.github/workflows/ci.yml`):
- Ubuntu: configure/build/test + smoke benchmark
- Windows(MSYS2 MinGW): configure/build/test + smoke benchmark
- 두 환경 모두 benchmark CSV 파싱 후 `bench_summary.txt` 생성
- benchmark 산출물(`bench_result.csv`, `bench_summary.txt`) 아티팩트 업로드

정적 점검 (`.github/workflows/quality.yml`):
- `cppcheck` 실행 (warning/style/performance/portability)

## Portfolio Metadata (Recommended)

GitHub `About`에 아래 값을 권장합니다.

- Description:
  - `C-based PQC crypto module lab integrating ML-KEM-768 and ML-DSA-65 with unified API, CLI, tests, benchmarks, and CI.`
- Topics:
  - `pqc`, `ml-kem`, `ml-dsa`, `cryptography`, `c`, `cmake`, `pqclean`
