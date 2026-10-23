// Clean conditions with standard if / else and function calls

fn max_value(a: int, b: int) -> int {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

fn main() -> int {
    let x = 40;
    let y = 70;
    let winner = max_value(x, y);

    return winner;
}
