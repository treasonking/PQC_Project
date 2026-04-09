# Benchmark

## Current Status

CLI benchmark는 `dummy`와 `mlkem-ref` 모두 지원하며, 콘솔 출력 + 파일 저장을 지원한다.

- `keygen`
- `encaps`
- `decaps`

## Command

```bash
pqc_cli benchmark --alg mlkem-ref --iterations 1000
pqc_cli benchmark --alg mlkem-ref --iterations 1000 --out bench_result.txt
pqc_cli benchmark --alg mlkem-ref --iterations 1000 --out bench_result.csv
```

CSV/TXT 결과 파싱:

```bash
python bench/parse_results.py bench_result.txt
python bench/parse_results.py bench_result.csv
```

## Output Policy

- 공유비밀 값 자체는 출력하지 않는다.
- 벤치마크 결과는 요약 통계만 기록한다.

## Measurement Method

- 동일 알고리즘에서 `iterations` 회 반복 후 평균 ms를 계산한다.
- 측정 항목: `keygen`, `encaps`, `decaps`
- 함께 기록되는 메타데이터:
  - algorithm name
  - key/ciphertext/shared-secret sizes
  - iteration count

## Interpretation Guide

- 같은 머신/같은 빌드 옵션에서 알고리즘/코드 변경 전후 비교에 사용한다.
- 단일 측정값보다는 여러 번 실행한 평균/분산을 같이 보는 것을 권장한다.
