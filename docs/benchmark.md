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
