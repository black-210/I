system FixedArithmetic;

routine main() -> u32 {
    // Explicit overflow semantics:
    // +! checked (traps on overflow)
    // +% modular wrapping
    // +? saturating
    bind max_u8: u32 = 255;
    mut wrapped: u32 = max_u8 +% 1; // wraps cleanly to 256 or 0

    // Static assertion at compile-time
    verify (256 == 0x100);

    yield wrapped;
}
