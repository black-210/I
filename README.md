# Language `I`: Pure Binary Systems Programming

```
   ___ 
  |_ _|
   | |    The Pure Binary Systems Language
  |___|   Deterministic Machine Semantics • Zero Web • Native 01 Bitstream
```

[![Language](https://img.shields.io/badge/Language-I-00f0ff.svg)](#)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Architecture](https://img.shields.io/badge/Target-x86__64--linux%20%7C%20baremetal-success.svg)](#)
[![Compiler](https://img.shields.io/badge/Compiler-Native%20ELF64%20%2B%2001%20Bitstream-brightgreen.svg)](#)
[![Documentation](https://img.shields.io/badge/Docs-Complete%20Manual%20%26%20Spec-blueviolet.svg)](docs/I_COMPLETE_REFERENCE.md)

> 📖 **للتوثيق الكامل والشامل باللغتين العربية والإنجليزية مع كافة الأكواد والشروحات:**  
> راجع الموسوعة الشاملة للغة في: [`docs/I_COMPLETE_REFERENCE.md`](docs/I_COMPLETE_REFERENCE.md)

---

## 1. What is `I`?

**`I`** is an independent, hardware-oriented, binary-first systems programming language. It is engineered from first principles for operating systems, microkernels, bare-metal bootloaders, device drivers, computer architecture research, defensive cybersecurity, and deterministic low-level computing.

The core principle of `I`:
> **The programmer must know with absolute determinism what the machine will execute, which bits and registers are manipulated, what memory offsets are accessed, and what physical risks exist.**

---

## 2. Core Technical Foundations

1. **Explicit Fixed Bit Widths & Sequences:**
   - Raw bitvectors: `b8`, `b16`, `b32`, `b64`, `b128`
   - Fixed-width unsigned integers: `u8`, `u16`, `u32`, `u64`
   - Fixed-width signed integers: `i8`, `i16`, `i32`, `i64`
   - No silent coercion, no hidden widening, no implicit narrowing.

2. **Explicit Arithmetic & Overflow Semantics:**
   - Checked arithmetic: `+!`, `-!`, `*!` (traps on overflow)
   - Modular wrapping arithmetic: `+%`, `-%`, `*%` (guaranteed two's complement wrap)
   - Saturating arithmetic: `+?`, `-?` (clamps at integer boundaries)

3. **Packed Binary Layouts & Bitfields:**
   - Memory structures with explicit bit and byte offsets:
     ```
     layout PciConfiguration {
         vendor_id:   u16 @ 0x00,
         device_id:   u16 @ 0x02,
         command_reg: u16 @ 0x04,
         status_reg:  u16 @ 0x06,
     }
     ```

4. **Hardware & Capability-Based Access Control:**
   - Unprivileged application code cannot access hardware ports or invoke raw memory operations without explicit compile-time capabilities:
     ```
     system KernelDriver;
     require cap: hardware_io;
     require cap: kernel_ring0;
     ```

5. **Native Code Generation & Pure 01 Bitstream:**
   - Emits System V AMD64 ELF64 executables directly.
   - Emits bare-metal flat `.bin` binaries with 0x55AA MBR boot signatures.
   - Emits pure bitstream representations (`.01` files) containing raw machine bits:
     ```
     [0x0000 | bit 00000]: 01001000  # 0x48 REX.W
     [0x0001 | bit 00008]: 00110001  # 0x31 XOR
     [0x0002 | bit 00016]: 11101101  # 0xED RBP, RBP
     ```

---

## 3. Toolchain & CLI Usage

Build the native compiler from source:
```bash
make
```

### CLI Subcommands:
- **Compile to native Linux ELF64 binary:**
  ```bash
  ./bin/i compile program.i -o program
  ./program
  ```
- **Direct Run (compile and execute):**
  ```bash
  ./bin/i run examples/beginner/01_first_program.i
  ```
- **Compile to Bare-Metal Flat Binary & 01 Bitstream:**
  ```bash
  ./bin/i compile kernel.i -o kernel.bin --target baremetal --bootsector --dump-01
  ```
- **Inspect Machine Code & Bitstream:**
  ```bash
  ./bin/i inspect examples/beginner/02_binary_and_bits.i
  ```
- **Run Language Self-Test Suite:**
  ```bash
  ./bin/i test
  ```
- **View Architectural Target Matrix:**
  ```bash
  ./bin/i targets
  ```

---

## 4. Repository Structure

```
├── README.md               # Main language documentation
├── LICENSE                 # GNU General Public License v3.0
├── SECURITY.md             # Security policy and disclosure
├── CONTRIBUTING.md         # Contribution guidelines
├── CODE_OF_CONDUCT.md      # Code of conduct
├── CHANGELOG.md            # Release log
├── ROADMAP.md              # Long-term language evolution
├── AUTHORS.md              # Authors and contributors
├── Makefile                # Toolchain build system
├── bin/
│   └── i                   # Native compiled compiler executable
├── compiler/
│   └── src/
│       ├── diagnostics.h/c # Spans and error emitter
│       ├── lexer.h/c       # Lexical analyzer
│       ├── ast.h/c         # Abstract Syntax Tree
│       ├── parser.h/c      # Recursive descent parser
│       ├── semantics.h/c   # Typechecker & capability validator
│       ├── x86_64.h/c      # Machine code encoder & backend
│       ├── elf64.h/c       # System V ELF64 binary writer
│       ├── rawbin.h/c      # Flat binary & 01 bitstream writer
│       ├── disasm.h/c      # Native x86-64 disassembler
│       └── main.c          # CLI entrypoint
├── docs/                   # Architectural & specification documents
│   ├── LANGUAGE.md         # Full language specification
│   ├── GRAMMAR.md          # Formal grammar specification
│   ├── BINARY_MODEL.md     # Bit representation & memory model
│   ├── HARDWARE.md         # MMIO, Ports, and Interrupt model
│   ├── SECURITY_MODEL.md   # Capabilities and defensive computing
│   └── TARGET_SUPPORT.md   # Supported CPU architecture matrix
└── examples/               # Real working programs in I
    ├── beginner/           # First steps, bits, and arithmetic
    ├── binary/             # Network packets and endian swaps
    ├── standalone/         # Bootloaders and kernels
    ├── hardware/           # UART serial driver and ports
    ├── security/           # Constant-time crypto comparisons
    ├── dsp/                # Fixed-point FIR filtering
    └── ai/                 # Quantized INT8 tensor dot product
```

---

## 5. License
Licensed under the GNU General Public License v3.0 (GPL-3.0). See [LICENSE](LICENSE) for details.
