# Architecture

## Goal

개인 프로젝트 수준에서 PQC 모듈의 공통 API, 오류 처리, 테스트 가능한 구조를 만드는 것을 목표로 한다.

## Layering

- `include/pqc_module.h`
  - 외부에 공개되는 API
  - 상태 코드, 알고리즘 선택, KEM 함수 원형
- `src/pqc_module.c`
  - 공통 API 진입점
  - 활성 백엔드로 라우팅
- `src/pqc_kem.c`
  - 알고리즘 종속 계층
  - 현재 `dummy` 구현 + `mlkem-ref` 연결용 placeholder
- `src/pqc_utils.c`
  - 보안 메모리 정리(`secure_memzero`), 상태 문자열 변환
- `src/main.c`
  - CLI 파서 및 파일 I/O 흐름

## Backend Strategy

- 기본 백엔드: `PQC_ALG_ML_KEM_768_DUMMY`
- 예정 백엔드: `PQC_ALG_ML_KEM_768_REF`
- 향후 확장: ML-DSA 등 추가 시 동일한 패턴으로 분리

## Security Rules

- 로그에 비밀키/공유비밀 바이트를 출력하지 않는다.
- 실패 경로에서 민감 버퍼를 지운다.
- API는 항상 상태코드(`pqc_status_t`)로 실패 원인을 반환한다.
