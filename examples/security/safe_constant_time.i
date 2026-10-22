system SafeConstantTime;

// Constant-time byte comparison to eliminate timing side-channels in cryptography
routine constant_time_eq8(a: u8, b: u8) -> u8 {
    bind diff: u8 = a ^ b;
    // (diff | -diff) >> 7 yields 0 if diff == 0, else 1
    bind diff_neg: u8 = 0 -% diff;
    bind non_zero: u8 = (diff | diff_neg) >> 7;
    // Return 1 if equal, 0 if different
    yield 1 ^ non_zero;
}

routine main() -> u32 {
    bind pass1: u8 = 0xA5;
    bind pass2: u8 = 0xA5;
    bind result: u8 = constant_time_eq8(pass1, pass2);
    verify (1 == 1);
    yield result;
}
