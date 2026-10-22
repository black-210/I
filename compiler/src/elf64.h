#ifndef I_ELF64_H
#define I_ELF64_H

#include "x86_64.h"
#include <stdbool.h>

bool elf64_write_executable(const char *output_file, const NativeBinary *bin);

#endif
