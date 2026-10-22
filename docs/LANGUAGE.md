# Language Specification: `I`

## 1. Grammatical Constructs

### System Units
Every compilation unit begins with an optional `system` statement and optional capability declarations:
```i
system MyUnit;

require cap: host_syscall;
require cap: hardware_io;
```

### Types
- `b8`, `b16`, `b32`, `b64`: Pure bit sequences.
- `u8`, `u16`, `u32`, `u64`: Unsigned machine words.
- `i8`, `i16`, `i32`, `i64`: Signed machine words.
- `void`: Unit / empty type.
- `*raw`: Opaque machine pointer.

### Bindings
- `bind name: type = expr;` (Immutable value)
- `mut name: type = expr;` (Mutable variable)

### Control Flow
- Conditionals:
  ```i
  branch (x > 10) {
      yield 1;
  } otherwise {
      yield 0;
  }
  ```
- Cycles (Loops):
  ```i
  cycle (mut i: u32 = 0; i < 10; i = i +% 1) {
      // Loop body
  }
  ```
- Repeat-Until:
  ```i
  repeat {
      step();
  } until (ready == 1);
  ```

### Static Verification
- `verify (const_expr);` Evaluated at compile-time. If false, compilation halts with error `S0008`.

### Packed Memory Layouts
```i
layout Descriptor {
    limit_low:  u16 @ 0x00,
    base_low:   u16 @ 0x02,
    base_mid:   u8  @ 0x04,
    access:     u8  @ 0x05,
    granularity:u8  @ 0x06,
    base_high:  u8  @ 0x07,
}
```
