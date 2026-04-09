# Architecture

## Goal

개인 프로젝트 수준에서 PQC 모듈의 공통 API, 오류 처리, 테스트 가능한 구조를 만드는 것을 목표로 한다.

## Why This Architecture

- 공통 API를 먼저 고정하면 알고리즘 구현을 교체해도 CLI/테스트 코드를 크게 바꾸지 않아도 된다.
- 알고리즘 종속 코드와 공통 코드를 분리하면 ML-KEM 이후 ML-DSA 확장 시 변경 범위가 줄어든다.
- 실패 처리를 상태코드 기반으로 통일하면 호출부에서 예외 흐름을 예측하기 쉬워진다.

## Layering

- `include/pqc_module.h`
  - 외부에 공개되는 API
  - 상태 코드, 알고리즘 선택, KEM 함수 원형
- `src/pqc_module.c`
  - 공통 API 진입점
  - 활성 백엔드로 라우팅
- `src/pqc_kem.c`
  - 알고리즘 종속 계층
  - 현재 `dummy` 구현 + `mlkem-ref(PQClean ML-KEM-768 clean)` 구현
- `src/pqc_utils.c`
  - 보안 메모리 정리(`secure_memzero`), 상태 문자열 변환
- `src/main.c`
  - CLI 파서 및 파일 I/O 흐름

## Backend Strategy

- 기본 백엔드: `PQC_ALG_ML_KEM_768_DUMMY`
- 참조 백엔드: `PQC_ALG_ML_KEM_768_REF` (PQClean)
- 향후 확장: ML-DSA 등 추가 시 동일한 패턴으로 분리

## Security Rules

- 로그에 비밀키/공유비밀 바이트를 출력하지 않는다.
- 실패 경로에서 민감 버퍼를 지운다.
- API는 항상 상태코드(`pqc_status_t`)로 실패 원인을 반환한다.

## Commercial Gap

이 프로젝트는 학습용/포트폴리오용 구조화에 초점을 둔다. 상용 모듈과의 주요 차이는 다음과 같다.

- FIPS/KCMVP 같은 공식 인증 요구사항을 충족하지 않는다.
- HSM, 키 수명주기, 감사추적, 접근통제 같은 운영 통제가 포함되지 않는다.
- side-channel 하드닝과 정형 검증은 범위 밖이다.
