# third_party layout

This directory is reserved for external PQC reference implementations.

Current layout:

```text
third_party/
  mlkem_pqclean/
    clean/      # PQClean ML-KEM-768 clean implementation files
    common/     # required common primitives (fips202, randombytes)
```

Integration policy:

- Keep upstream sources as untouched as possible.
- Put adapter glue in `src/pqc_kem.c` (or `src/pqc_kem_mlkem_ref.c` later).
- Do not log secrets, private keys, or raw shared secret bytes.
