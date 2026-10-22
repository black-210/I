# Binary Model & Bitstream Architecture of `I`

## 1. Pure 01 Bitstream Philosophy
`I` treats bits and machine instructions as first-class geometric objects:
- Every instruction emitted by the backend can be outputted as a literal 01 ASCII bitstream using `--dump-01` or `--format bits01`.
- The compiler outputs byte and bit offsets for every emitted instruction, allowing direct verification down to individual transistor-level gate transitions and register states.

## 2. Instruction Encoding in x86-64 Backend
- Fixed prefix generation: REX.W (`0x48`), REX.B, REX.R, REX.X.
- Direct ModR/M and SIB encoding without intermediate assembler or external runtime.
- System V ABI calling convention:
  - Integer arguments: `RDI`, `RSI`, `RDX`, `RCX`, `R8`, `R9`.
  - Return register: `RAX`.
  - Callee-saved: `RBX`, `RSP`, `RBP`, `R12`, `R13`, `R14`, `R15`.
- Linux Syscall Convention:
  - System call number in `RAX`.
  - Arguments in `RDI`, `RSI`, `RDX`, `R10`, `R8`, `R9`.
  - Instruction: `syscall` (`0x0F 0x05`).

## 3. Bare-Metal & Bootloader Representation
- Raw flat binaries omit all container headers (ELF, PE, Mach-O).
- Entry point can be aligned at:
  - `0x7C00` (Legacy BIOS MBR bootsector) with signature `0x55 0xAA` at offset 510.
  - `0x100000` (1MB Physical High Memory Kernel Entry).
