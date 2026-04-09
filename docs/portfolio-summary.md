# Portfolio Summary

## Project Overview

PQC 알고리즘을 직접 모듈화하고 CLI/테스트/벤치마크/문서화를 포함해 개발 역량을 증명하는 개인 프로젝트.

## Why PQC

- 양자 컴퓨팅 시대 대비 암호 전환 흐름을 실무 관점으로 학습
- 알고리즘 자체 발명보다 모듈 인터페이스/실패 처리/검증 자동화 경험 확보

## Why ML-KEM first

- KEM 흐름(키생성/캡슐화/복호화)이 명확해 구조 학습에 적합
- 이후 서명(ML-DSA) 확장을 위한 기반을 만들기 좋음

## Engineering Focus

- 공통 API 설계 및 상태코드 일관성
- 민감정보 로깅 방지
- 테스트(정상/예외) 구조화
- 벤치마크 수집 파이프라인 설계

## Error Handling Approach

- 모든 핵심 함수는 `pqc_status_t`를 반환해 호출부에서 실패 유형을 분기할 수 있게 했다.
- 입력 포인터/버퍼 길이를 우선 검증하고, 조건 불일치 시 즉시 오류 반환한다.
- decapsulation 실패 시 공유비밀 버퍼를 정리하고 실패 상태를 반환한다.

## Sensitive Data Logging Policy

- 비밀키/공유비밀 원문은 stdout/stderr에 출력하지 않는다.
- 에러 메시지는 상태코드와 파일 입출력 실패 여부 중심으로만 기록한다.
- 민감 버퍼는 `secure_memzero`를 사용해 해제 전 정리한다.

## Performance Summary (Example)

- 환경: Windows + w64devkit + CMake(Release)
- 알고리즘: ML-KEM-768-PQCLEAN
- 예시 측정(100회):
  - keygen: 0.540 ms
  - encaps: 0.500 ms
  - decaps: 0.080 ms

## Difference from Production Crypto Modules

- 인증 체계(FIPS/KCMVP), 운영 절차, 키 관리 인프라가 없다.
- 보안 검증(침투/측채널/정형기법) 범위가 제한적이다.
- 목표는 상용 배포가 아니라 구조 이해와 엔지니어링 역량 증명이다.

## Limitations (Honest)

- 현재 단계는 dummy + ML-KEM 참조구현 연동 상태
- 상용 수준 하드닝/인증(KCMVP 등)은 범위 밖

## Next Plan

- ML-DSA 백엔드 추가
- CI 자동화
- Linux/Windows 교차 벤치마크 리포트 표준화
