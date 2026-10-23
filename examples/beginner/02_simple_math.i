// Clean arithmetic: simple variables (let = constant, var = mutable)

fn main() -> int {
    let base_price = 100;
    var tax = 15;
    var total = base_price + tax;

    // Static verification check
    assert (100 + 15 == 115);

    return total;
}
