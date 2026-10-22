#ifndef I_RAWBIN_H
#define I_RAWBIN_H

#include "x86_64.h"
#include <stdbool.h>

bool rawbin_write_flat(const char *output_file, const NativeBinary *bin, bool bootsector);
bool rawbin_write_bits01(const char *output_file, const NativeBinary *bin);
bool rawbin_write_hex(const char *output_file, const NativeBinary *bin);

#endif
