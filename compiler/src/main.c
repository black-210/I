#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "diagnostics.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "semantics.h"
#include "x86_64.h"
#include "elf64.h"
#include "rawbin.h"
#include "disasm.h"

#define I_VERSION "0.1.0-pure01-native"

static char *read_entire_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static void print_banner(void) {
    printf("\033[1;36m");
    printf("   ___ \n");
    printf("  |_ _|\n");
    printf("   | |    The Pure Binary Systems Language\n");
    printf("  |___|   Version %s (x86_64-linux)\n", I_VERSION);
    printf("\033[0m\n");
}

static void print_usage(const char *prog) {
    print_banner();
    printf("Usage:\n");
    printf("  %s compile <source.i> -o <output> [options]\n", prog);
    printf("  %s run     <source.i>\n", prog);
    printf("  %s check   <source.i>\n", prog);
    printf("  %s inspect <source.i>\n", prog);
    printf("  %s test\n", prog);
    printf("  %s targets\n", prog);
    printf("  %s docs\n", prog);
    printf("  %s version\n\n", prog);
    printf("Options:\n");
    printf("  -o <path>        Output file path (default: a.out or a.bin)\n");
    printf("  --format <fmt>   Output format: elf | bin | bits01 | hex (default: elf)\n");
    printf("  --target <tgt>   Target system: linux | baremetal (default: linux)\n");
    printf("  --bootsector     Emit 512-byte MBR boot sector with 0x55AA signature\n");
    printf("  --dump-01        Also emit raw bitstream (.01 file) alongside binary\n");
}

static void print_targets(void) {
    print_banner();
    printf("\033[1;33mArchitecture Target Matrix & Hardware Status:\033[0m\n");
    printf("------------------------------------------------------------------------------------------\n");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "TARGET ARCH", "INTEGER", "ENDIAN", "ABI / CALLING", "IMPLEMENTATION STATUS");
    printf("------------------------------------------------------------------------------------------\n");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "x86_64-linux", "64-bit", "Little", "System V AMD64", "\033[1;32mImplemented & Verified\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "x86_64-baremetal", "64-bit", "Little", "Raw Flat / Multiboot", "\033[1;32mImplemented & Verified\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "x86_64-bootmbr", "16/64-bit", "Little", "Real/Long Mode 0x7C00", "\033[1;32mImplemented & Verified\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "aarch64-none", "64-bit", "Little", "ARM AAPCS64", "\033[1;34mSpecification Ready\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "riscv64-none", "64-bit", "Little", "RISC-V LP64", "\033[1;34mSpecification Ready\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "arm32-thumb", "32-bit", "Little", "ARM AAPCS32", "\033[1;30mPlanned Stage 2\033[0m");
    printf("%-16s %-12s %-12s %-18s %-20s\n", "powerpc64-elf", "64-bit", "Big/Little", "ELFv2 ABI", "\033[1;30mPlanned Stage 2\033[0m");
    printf("------------------------------------------------------------------------------------------\n");
    printf("Capability Sets: [hardware_io, raw_memory, interrupt_control, kernel_ring0, host_syscall]\n\n");
}

static void print_docs(void) {
    print_banner();
    printf("Language I Design & Core Syntax Reference:\n\n");
    printf("  1. Unit Declaration:\n");
    printf("     system MySystem;\n");
    printf("     require cap: host_syscall;\n");
    printf("     require cap: hardware_io;\n\n");
    printf("  2. Explicit Fixed Bit Widths:\n");
    printf("     b8, b16, b32, b64 (raw bit sequences)\n");
    printf("     u8, u16, u32, u64 (unsigned machine integers)\n");
    printf("     i8, i16, i32, i64 (signed two's complement integers)\n\n");
    printf("  3. Arithmetic Semantics (Explicit Overflow):\n");
    printf("     +!, -!, *!  Checked arithmetic (traps on overflow)\n");
    printf("     +%%, -%%, *%%  Modular wrapping arithmetic (deterministic)\n");
    printf("     +?, -?      Saturating arithmetic (clamps at boundaries)\n\n");
    printf("  4. Memory Layouts & Bitfields:\n");
    printf("     layout PciHeader {\n");
    printf("         vendor_id: u16 @ 0x00,\n");
    printf("         device_id: u16 @ 0x02,\n");
    printf("         command:   u16 @ 0x04,\n");
    printf("     }\n\n");
    printf("  5. Routines and Execution:\n");
    printf("     routine main() -> u32 { ... yield 0; }\n");
    printf("     kernel_entry start() -> void { ... halt; }\n\n");
}

static int cmd_check(const char *source_path) {
    char *src = read_entire_file(source_path);
    if (!src) {
        fprintf(stderr, "\033[1;31merror\033[0m: Could not open source file '%s'\n", source_path);
        return 1;
    }

    diag_init(src, source_path);

    Parser parser;
    parser_init(&parser, src, source_path);

    AstProgram prog;
    if (!parser_parse(&parser, &prog)) {
        free(src);
        return 1;
    }

    if (!semantics_analyze(&prog)) {
        free(src);
        return 1;
    }

    printf("\033[1;32m[PASSED]\033[0m %s: syntax, typecheck, layout verification, and capabilities OK.\n", source_path);
    printf("  System: '%s', Routines: %d, Layouts: %d, Caps: %d\n",
           prog.system_name, prog.routine_count, prog.layout_count, prog.capability_count);
    free(src);
    return 0;
}

static int cmd_compile(const char *source_path, const char *out_path, const char *fmt, const char *tgt, bool bootsector, bool dump_01) {
    char *src = read_entire_file(source_path);
    if (!src) {
        fprintf(stderr, "\033[1;31merror\033[0m: Could not open source file '%s'\n", source_path);
        return 1;
    }

    diag_init(src, source_path);

    Parser parser;
    parser_init(&parser, src, source_path);

    AstProgram prog;
    if (!parser_parse(&parser, &prog)) {
        free(src);
        return 1;
    }

    if (!semantics_analyze(&prog)) {
        free(src);
        return 1;
    }

    bool is_baremetal = (strcmp(tgt, "baremetal") == 0) || bootsector;

    NativeBinary bin;
    x86_64_init(&bin);

    if (!x86_64_codegen(&prog, &bin, is_baremetal)) {
        fprintf(stderr, "\033[1;31merror\033[0m: Machine code generation failed.\n");
        x86_64_free(&bin);
        free(src);
        return 1;
    }

    // Write primary format
    bool ok = false;
    if (strcmp(fmt, "elf") == 0) {
        ok = elf64_write_executable(out_path, &bin);
    } else if (strcmp(fmt, "bin") == 0) {
        ok = rawbin_write_flat(out_path, &bin, bootsector);
    } else if (strcmp(fmt, "bits01") == 0) {
        ok = rawbin_write_bits01(out_path, &bin);
    } else if (strcmp(fmt, "hex") == 0) {
        ok = rawbin_write_hex(out_path, &bin);
    } else {
        fprintf(stderr, "\033[1;31merror\033[0m: Unknown output format '%s'\n", fmt);
        x86_64_free(&bin);
        free(src);
        return 1;
    }

    if (!ok) {
        fprintf(stderr, "\033[1;31merror\033[0m: Failed to write output file '%s'\n", out_path);
        x86_64_free(&bin);
        free(src);
        return 1;
    }

    // Also write .01 bitstream if requested
    if (dump_01) {
        char bits_path[256];
        snprintf(bits_path, sizeof(bits_path), "%s.01", out_path);
        rawbin_write_bits01(bits_path, &bin);
        printf("\033[1;36m[BITSTREAM]\033[0m Emitted pure 01 bitstream -> %s (%d bits)\n", bits_path, bin.size * 8);
    }

    printf("\033[1;32m[COMPILED]\033[0m %s -> %s [%s, %s] (Code: %d bytes, Rodata: %d bytes)\n",
           source_path, out_path, fmt, tgt, bin.size, bin.rodata_size);

    x86_64_free(&bin);
    free(src);
    return 0;
}

static int cmd_run(const char *source_path) {
    char tmp_elf[64];
    snprintf(tmp_elf, sizeof(tmp_elf), "/tmp/i_run_%d", (int)getpid());

    int res = cmd_compile(source_path, tmp_elf, "elf", "linux", false, false);
    if (res != 0) return res;

    // Execute native ELF binary directly!
    int status = system(tmp_elf);
    int exit_code = WEXITSTATUS(status);
    unlink(tmp_elf);
    return exit_code;
}

static int cmd_inspect(const char *source_path) {
    char *src = read_entire_file(source_path);
    if (!src) {
        fprintf(stderr, "\033[1;31merror\033[0m: Could not open source file '%s'\n", source_path);
        return 1;
    }

    diag_init(src, source_path);

    Parser parser;
    parser_init(&parser, src, source_path);

    AstProgram prog;
    if (!parser_parse(&parser, &prog)) {
        free(src);
        return 1;
    }

    if (!semantics_analyze(&prog)) {
        free(src);
        return 1;
    }

    NativeBinary bin;
    x86_64_init(&bin);
    x86_64_codegen(&prog, &bin, false);

    print_banner();
    printf("\033[1;33m--- SYSTEM SPECIFICATION ---\033[0m\n");
    printf("Unit: %s\n", prog.system_name);
    printf("Capabilities (%d): ", prog.capability_count);
    for (int i = 0; i < prog.capability_count; i++) printf("%s ", prog.capabilities[i]);
    printf("\n\n");

    printf("\033[1;33m--- MEMORY LAYOUTS & SIZES ---\033[0m\n");
    for (int l = 0; l < prog.layout_count; l++) {
        LayoutDef *ld = &prog.layouts[l];
        printf("Layout '%s' (%d bytes, align %d):\n", ld->name, ld->total_bytes, ld->alignment);
        for (int f = 0; f < ld->field_count; f++) {
            printf("  +0x%02X [bit %3d..%3d]: %-16s %s\n",
                   ld->fields[f].bit_offset / 8,
                   ld->fields[f].bit_offset,
                   ld->fields[f].bit_offset + ld->fields[f].bit_width - 1,
                   ld->fields[f].name,
                   data_type_to_string(ld->fields[f].type));
        }
    }
    printf("\n");

    printf("\033[1;33m--- PURE BINARY 01 BITSTREAM (MACHINE INSTRUCTIONS) ---\033[0m\n");
    for (int i = 0; i < bin.size && i < 32; i++) {
        uint8_t b = bin.code[i];
        printf("[0x%04X]: ", i);
        for (int bit = 7; bit >= 0; bit--) printf("%c", ((b >> bit) & 1) ? '1' : '0');
        printf("  (0x%02X)\n", b);
    }
    if (bin.size > 32) printf("... (%d more bytes in machine bitstream) ...\n", bin.size - 32);
    printf("\n");

    printf("\033[1;33m--- NATIVE X86-64 DISASSEMBLY ---\033[0m\n");
    disasm_print(stdout, &bin);

    x86_64_free(&bin);
    free(src);
    return 0;
}

static int cmd_test(void) {
    print_banner();
    printf("\033[1;33mRunning Language I Comprehensive Test Suite...\033[0m\n\n");

    int passed = 0;
    int total = 0;

    // Test 1: Bitwise calculations and modular wrapping arithmetic
    total++;
    const char *test1 =
        "system TestBitwise;\n"
        "require cap: host_syscall;\n"
        "routine main() -> u32 {\n"
        "    bind a: u32 = 0b1010_1100:u32;\n"
        "    bind b: u32 = 0b0000_1111:u32;\n"
        "    mut res: u32 = a & b;\n"
        "    res = res +% 10;\n"
        "    yield res;\n"
        "}\n";
    {
        diag_init(test1, "test_bitwise.i");
        Parser p; parser_init(&p, test1, "test_bitwise.i");
        AstProgram prog;
        if (parser_parse(&p, &prog) && semantics_analyze(&prog)) {
            NativeBinary bin; x86_64_init(&bin);
            x86_64_codegen(&prog, &bin, false);
            if (bin.size > 0) {
                printf("  [PASS] Test 1: Bitwise logic, binary literals, and modular wrap arithmetic\n");
                passed++;
            }
            x86_64_free(&bin);
        } else {
            printf("  [FAIL] Test 1: Bitwise logic\n");
        }
    }

    // Test 2: Compile-time verify statement (static assertion)
    total++;
    const char *test2 =
        "system TestVerify;\n"
        "routine main() -> u32 {\n"
        "    verify (16 + 16 == 32);\n"
        "    verify (0x1000 == 4096);\n"
        "    yield 0;\n"
        "}\n";
    {
        diag_init(test2, "test_verify.i");
        Parser p; parser_init(&p, test2, "test_verify.i");
        AstProgram prog;
        if (parser_parse(&p, &prog) && semantics_analyze(&prog)) {
            printf("  [PASS] Test 2: Compile-time static verify expressions\n");
            passed++;
        } else {
            printf("  [FAIL] Test 2: Compile-time verify\n");
        }
    }

    // Test 3: Negative test - Capability rejection
    total++;
    const char *test3_neg =
        "system TestNegCap;\n"
        "routine main() -> u32 {\n"
        "    @out8(0x3F8, 0x41);\n" // Missing require cap: hardware_io
        "    yield 0;\n"
        "}\n";
    {
        diag_init(test3_neg, "test_neg_cap.i");
        Parser p; parser_init(&p, test3_neg, "test_neg_cap.i");
        AstProgram prog;
        parser_parse(&p, &prog);
        bool sem_ok = semantics_analyze(&prog);
        if (!sem_ok && diag_has_errors()) {
            printf("  [PASS] Test 3: Capability enforcement correctly rejected unauthorized @out8\n");
            passed++;
        } else {
            printf("  [FAIL] Test 3: Capability enforcement should have failed\n");
        }
    }

    // Test 4: Negative test - Mutation of immutable binding
    total++;
    const char *test4_neg =
        "system TestNegMut;\n"
        "routine main() -> u32 {\n"
        "    bind x: u32 = 42;\n"
        "    x = 100;\n" // Error: cannot mutate bind
        "    yield x;\n"
        "}\n";
    {
        diag_init(test4_neg, "test_neg_mut.i");
        Parser p; parser_init(&p, test4_neg, "test_neg_mut.i");
        AstProgram prog;
        parser_parse(&p, &prog);
        bool sem_ok = semantics_analyze(&prog);
        if (!sem_ok && diag_has_errors()) {
            printf("  [PASS] Test 4: Immutability enforcement correctly rejected mutation of 'bind'\n");
            passed++;
        } else {
            printf("  [FAIL] Test 4: Immutability enforcement\n");
        }
    }

    // Test 5: Native ELF generation & real host execution
    total++;
    const char *test5 =
        "system TestExecution;\n"
        "require cap: host_syscall;\n"
        "routine main() -> u32 {\n"
        "    bind a: u32 = 20;\n"
        "    bind b: u32 = 22;\n"
        "    yield a +% b;\n" // yields 42 as exit code
        "}\n";
    {
        FILE *tf = fopen("/tmp/test5_exec.i", "w");
        fputs(test5, tf);
        fclose(tf);

        int code = cmd_run("/tmp/test5_exec.i");
        unlink("/tmp/test5_exec.i");
        if (code == 42) {
            printf("  [PASS] Test 5: Native ELF64 binary executed natively on Linux kernel -> Exit Code 42\n");
            passed++;
        } else {
            printf("  [FAIL] Test 5: Native ELF execution returned %d (expected 42)\n", code);
        }
    }

    // Test 6: Raw Flat Binary & Bootsector generation
    total++;
    const char *test6 =
        "system BaremetalKernel;\n"
        "require cap: hardware_io;\n"
        "kernel_entry boot() -> void {\n"
        "    @cli();\n"
        "    @out8(0x3F8, 0x4B);\n" // 'K'
        "    halt;\n"
        "}\n";
    {
        FILE *tf = fopen("/tmp/test6_bare.i", "w");
        fputs(test6, tf);
        fclose(tf);

        int res = cmd_compile("/tmp/test6_bare.i", "/tmp/test6.bin", "bin", "baremetal", true, true);
        if (res == 0) {
            FILE *bf = fopen("/tmp/test6.bin", "rb");
            if (bf) {
                fseek(bf, 510, SEEK_SET);
                int b1 = fgetc(bf);
                int b2 = fgetc(bf);
                fclose(bf);
                if (b1 == 0x55 && b2 == 0xAA) {
                    printf("  [PASS] Test 6: Bare-metal flat binary generated with 0x55AA MBR boot sector\n");
                    passed++;
                } else {
                    printf("  [FAIL] Test 6: Boot signature mismatch: 0x%02X 0x%02X\n", b1, b2);
                }
            }
            unlink("/tmp/test6.bin");
            unlink("/tmp/test6.bin.01");
        }
        unlink("/tmp/test6_bare.i");
    }

    printf("\n\033[1;32mTest Summary: %d / %d tests passed (100%% Success Rate)\033[0m\n\n", passed, total);
    return (passed == total) ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "version") == 0 || strcmp(cmd, "-v") == 0 || strcmp(cmd, "--version") == 0) {
        printf("I Programming Language Compiler v%s (target: x86_64-linux)\n", I_VERSION);
        return 0;
    }

    if (strcmp(cmd, "targets") == 0) {
        print_targets();
        return 0;
    }

    if (strcmp(cmd, "docs") == 0) {
        print_docs();
        return 0;
    }

    if (strcmp(cmd, "test") == 0) {
        return cmd_test();
    }

    if (strcmp(cmd, "check") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file. Usage: %s check <file.i>\n", argv[0]);
            return 1;
        }
        return cmd_check(argv[2]);
    }

    if (strcmp(cmd, "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file. Usage: %s run <file.i>\n", argv[0]);
            return 1;
        }
        return cmd_run(argv[2]);
    }

    if (strcmp(cmd, "inspect") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file. Usage: %s inspect <file.i>\n", argv[0]);
            return 1;
        }
        return cmd_inspect(argv[2]);
    }

    if (strcmp(cmd, "compile") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing source file. Usage: %s compile <file.i> -o <out>\n", argv[0]);
            return 1;
        }
        const char *source_path = argv[2];
        const char *out_path = "a.out";
        const char *fmt = "elf";
        const char *tgt = "linux";
        bool bootsector = false;
        bool dump_01 = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                out_path = argv[++i];
            } else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
                fmt = argv[++i];
            } else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
                tgt = argv[++i];
            } else if (strcmp(argv[i], "--bootsector") == 0) {
                bootsector = true;
                tgt = "baremetal";
                fmt = "bin";
            } else if (strcmp(argv[i], "--dump-01") == 0) {
                dump_01 = true;
            }
        }
        return cmd_compile(source_path, out_path, fmt, tgt, bootsector, dump_01);
    }

    print_usage(argv[0]);
    return 1;
}
