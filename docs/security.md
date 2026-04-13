# Security Engineering Notes

## Threat Model (Current Scope)

- 로컬 실행 환경에서 API/CLI 사용 시 민감 데이터 노출을 줄이는 것을 목표로 한다.
- 알고리즘 정확성과 실패 시 안전한 동작(크래시/노출 방지)에 우선순위를 둔다.

## What Is Implemented

- 민감 데이터 로그 출력 금지
- 에러 경로에서 상태코드 중심 처리
- 비밀 버퍼 `secure_memzero` 정리
- 파일 I/O 실패/버퍼 길이 오류 등 방어 로직

## Known Limits

- 더미 알고리즘은 `rand()` 사용으로 암호학적 안전성을 보장하지 않는다.
- constant-time 동작 전체를 보장하지 않는다.
- side-channel(타이밍/캐시/전력) 대응은 미구현
- 하드웨어 키 관리/HSM/감사추적은 범위 밖

## Secure Coding Checklist (Project Rule)

- 비밀값을 stdout/stderr에 출력하지 않는다.
- 반환값 검사 없이 진행하지 않는다.
- 길이 검증 후 메모리 접근한다.
- 실패 시 민감 버퍼를 정리한다.
- 테스트에 음성 케이스(손상/길이 오류/입력 누락)를 반드시 포함한다.
