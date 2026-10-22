system EndianSwap;

// Byte swap for 32-bit big-endian <-> little-endian conversion
routine swap32(v: u32) -> u32 {
    bind b0: u32 = (v & 0x000000FF) << 24;
    bind b1: u32 = (v & 0x0000FF00) << 8;
    bind b2: u32 = (v & 0x00FF0000) >> 8;
    bind b3: u32 = (v & 0xFF000000) >> 24;
    yield b0 | b1 | b2 | b3;
}

routine main() -> u32 {
    bind original: u32 = 0x12345678;
    bind swapped:  u32 = swap32(original);
    // Swapped should be 0x78563412
    yield 0;
}
