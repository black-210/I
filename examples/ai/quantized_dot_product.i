system QuantizedAiKernel;

// INT8 quantized neural network inference layer (GEMM / Dot Product accumulator)
routine dot_product_4(w0: i8, w1: i8, w2: i8, w3: i8, x0: i8, x1: i8, x2: i8, x3: i8) -> i32 {
    bind p0: i32 = w0 *% x0;
    bind p1: i32 = w1 *% x1;
    bind p2: i32 = w2 *% x2;
    bind p3: i32 = w3 *% x3;

    mut sum: i32 = p0 +% p1;
    sum = sum +% p2;
    sum = sum +% p3;

    yield sum;
}

// ReLU activation on quantized accumulator
routine relu(val: i32) -> i32 {
    branch (val > 0) {
        yield val;
    } otherwise {
        yield 0;
    }
}

routine main() -> u32 {
    bind acc: i32 = dot_product_4(2, 3, 4, -1, 10, 20, 5, 10);
    bind activated: i32 = relu(acc);
    yield 0;
}
