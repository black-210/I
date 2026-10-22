system BinaryAndBits;

routine main() -> u32 {
    // Exact binary bit literals
    bind reg_a: u32 = 0b1100_1010_0101_0011:u32;
    bind mask:  u32 = 0b0000_1111_0000_1111:u32;

    // Bitwise operations
    mut isolated: u32 = reg_a & mask;
    mut shifted:  u32 = isolated >> 4;
    mut rotated:  u32 = shifted <<< 2;

    yield rotated;
}
