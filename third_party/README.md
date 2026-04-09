# third_party layout

This directory is reserved for external PQC reference implementations.

Recommended layout for ML-KEM integration:

```text
third_party/
  mlkem/
    include/
    src/
    LICENSE
```

Integration policy:

- Keep upstream sources as untouched as possible.
- Put adapter glue in `src/pqc_kem.c` (or `src/pqc_kem_mlkem_ref.c` later).
- Do not log secrets, private keys, or raw shared secret bytes.
