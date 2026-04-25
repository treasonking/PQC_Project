# Security Engineering Notes

## Threat Model (Current Scope)

- 로컬 실행 환경에서 API/CLI 사용 시 민감 데이터 노출을 줄이는 것을 목표로 한다.
- 알고리즘 정확성과 실패 시 안전한 동작(크래시/노출 방지)에 우선순위를 둔다.
- 본 프로젝트는 학습/포트폴리오/참조 구현 통합 목적이며, production-ready 보안 제품을 목표로 하지 않는다.

## What Is Implemented

- 민감 데이터 로그 출력 금지
- 에러 경로에서 상태코드 중심 처리
- 비밀 버퍼 `secure_memzero` 정리
- 파일 I/O 실패/버퍼 길이 오류 등 방어 로직
- KAT 성격의 벡터 검증, 변조 입력, 길이 오류, 누락/손상 파일 실패 경로 테스트

## Known Limits

- production hardening, 인증 대응(FIPS/KCMVP), 운영 키관리(HSM/감사추적)는 범위 밖이다.
- 더미 알고리즘은 `rand()` 기반 테스트 RNG를 사용하므로 암호학적 안전성을 보장하지 않는다.
- 더미 백엔드는 API/CLI/테스트 흐름 검증 전용이며 실사용 금지이다.
- 활성 KEM/Signature 알고리즘 선택은 프로세스 전역 상태를 사용하므로 thread-safe를 보장하지 않는다.
- constant-time 동작 전체를 보장하지 않는다.
- side-channel(타이밍/캐시/전력) 대응은 미구현
- `secure_memzero`는 컴파일러 최적화 제거를 줄이기 위한 방어적 구현이지만, 플랫폼별 완전한 보안 삭제 보장은 별도 검증이 필요하다.

## Thread-Safety Note

현재 API는 `pqc_set_algorithm()`과 `pqc_set_signature_algorithm()`으로 활성 백엔드를 선택한다. 이 선택값은 전역 상태이므로 여러 스레드가 동시에 다른 알고리즘을 선택해 호출하는 사용 방식은 지원하지 않는다.

장기 개선 방향은 `pqc_context_t` 같은 명시적 컨텍스트를 도입해 알고리즘 선택과 설정을 호출자 소유 핸들로 이동하는 것이다. 이렇게 하면 CLI의 단순성은 유지하면서도 라이브러리 사용자는 스레드별 컨텍스트를 분리할 수 있다.

## Dummy Backend Warning

Dummy KEM/Signature 백엔드는 참조 구현 연결 전에도 API, CLI, 테스트, 오류 경로를 검증하기 위한 테스트 전용 백엔드이다. 내부 난수는 `rand()`/`srand(time(NULL))` 계열의 테스트 RNG이며 예측 가능성과 전역 상태 문제가 있으므로 키 생성, 공유 비밀, 서명 등 암호학적 보안 목적에 사용하면 안 된다.

## Secure Coding Checklist (Project Rule)

- 비밀값을 stdout/stderr에 출력하지 않는다.
- 반환값 검사 없이 진행하지 않는다.
- 길이 검증 후 메모리 접근한다.
- 실패 시 민감 버퍼를 정리한다.
- 테스트에 음성 케이스(손상/길이 오류/입력 누락)를 반드시 포함한다.
