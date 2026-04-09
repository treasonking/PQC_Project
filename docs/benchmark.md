# Benchmark

## Current Status

현재는 더미 KEM 구현 기준으로 CLI에서 평균 시간을 측정한다.

- `keygen`
- `encaps`
- `decaps`

## Command

```bash
pqc_cli benchmark --iterations 1000
```

## Planned Upgrade

실제 ML-KEM 참조 구현 연결 후 아래를 문서화한다.

- 키 생성 평균 시간
- 캡슐화/복호화 평균 시간
- 공개키/비밀키/ciphertext/shared secret 크기
- 환경 정보(OS, CPU, 컴파일러, 빌드 옵션)

## Output Policy

- 공유비밀 값 자체는 출력하지 않는다.
- 벤치마크 결과는 요약 통계만 기록한다.
