# Security Policy for Language I

## Security Philosophy
`I` is built from the ground up for safety-critical, operating system, and defensive cybersecurity operations:
1. **Capabilities Isolation:** Hardware port I/O, raw memory access, and direct interrupts cannot be executed by user code without explicit, compile-time capability tokens (`cap: hardware_io`, `cap: raw_memory`, `cap: kernel_ring0`).
2. **Deterministic Overflow:** No silent integer overflows; programs must explicitly specify checked arithmetic (`+!`, `-!`), modular wrapping (`+%`), or saturating math (`+?`).
3. **No Hidden Coercions:** There are zero implicit width conversions or pointer coercions.
4. **Reproducible Bitstream:** Every build produces an exact bitstream output for auditing and verification.

## Reporting Security Issues
To report a vulnerability in the `I` compiler or standard library, please open an issue or email security disclosures to `security@i-systems-lang.org`.
