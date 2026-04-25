# CLI Guide

## Normal Flow

KEM keygen/encaps/decaps:

```bash
./build/pqc_cli info --alg mlkem-ref --sig-alg mldsa-ref
./build/pqc_cli keygen --alg mlkem-ref --pub kem_pub.key --sec kem_sec.key
./build/pqc_cli encaps --alg mlkem-ref --pub kem_pub.key --ct kem.ct --ss sender.ss
./build/pqc_cli decaps --alg mlkem-ref --sec kem_sec.key --ct kem.ct --ss receiver.ss
```

Signature keygen/sign/verify:

```bash
./build/pqc_cli sig-keygen --sig-alg mldsa-ref --pub sig_pub.key --sec sig_sec.key
./build/pqc_cli sign --sig-alg mldsa-ref --sec sig_sec.key --msg message.txt --sig message.sig
./build/pqc_cli verify --sig-alg mldsa-ref --pub sig_pub.key --msg message.txt --sig message.sig
```

Benchmark:

```bash
./build/pqc_cli benchmark --alg mlkem-ref --sig-alg mldsa-ref --iterations 1000 --out bench_result.csv
python bench/parse_results.py bench_result.csv
```

## Failure Scenarios

| Scenario | Example | Expected behavior |
| --- | --- | --- |
| Missing public key | `encaps --pub does_not_exist.key` | fails with `failed to read public key file` |
| Corrupted KEM inputs | `decaps --sec tests/data/bad.sec --ct tests/data/bad.ct` | fails without printing secret material |
| Missing message | `sign --msg missing.txt` | fails with `failed to read message file` |
| Bad algorithm | `info --alg unknown` | fails with `unsupported --alg value` |
| Bad signature algorithm | `info --sig-alg unknown` | fails with `unsupported --sig-alg value` |

## Output Policy

- Secret keys, shared secrets, and raw signature internals are never printed.
- CLI errors report operation/status context only.
- Output files are raw binary artifacts sized according to the selected backend.
