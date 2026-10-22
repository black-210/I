#ifndef I_DISASM_H
#define I_DISASM_H

#include "x86_64.h"
#include <stdio.h>

void disasm_print(FILE *out, const NativeBinary *bin);

#endif
