#ifndef I_X86_64_H
#define I_X86_64_H

#include "ast.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *code;
    int size;
    int capacity;

    uint8_t *rodata;
    int rodata_size;
    int rodata_capacity;

    uint64_t vaddr_code;
    uint64_t vaddr_rodata;
    int entry_offset; // offset of main or kernel_entry in code
} NativeBinary;

void x86_64_init(NativeBinary *bin);
bool x86_64_codegen(AstProgram *prog, NativeBinary *bin, bool is_freestanding);
void x86_64_free(NativeBinary *bin);

#endif
