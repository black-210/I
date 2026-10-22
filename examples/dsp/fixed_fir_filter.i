system FixedFirFilter;

// 3-Tap Direct Form FIR filter with fixed-point Q15 arithmetic
routine fir_tap3(sample: i32, tap0: i32, tap1: i32, tap2: i32, c0: i32, c1: i32, c2: i32) -> i32 {
    bind term0: i32 = tap0 *% c0;
    bind term1: i32 = tap1 *% c1;
    bind term2: i32 = tap2 *% c2;

    mut acc: i32 = term0 +% term1;
    acc = acc +% term2;

    // Scale down fixed-point fraction (Q15: shift 15 bits)
    yield acc >> 15;
}

routine main() -> u32 {
    bind s: i32 = 1000;
    bind out_val: i32 = fir_tap3(s, 1000, 800, 600, 16384, 8192, 4096);
    yield 0;
}
