#include "rawbin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool rawbin_write_flat(const char *output_file, const NativeBinary *bin, bool bootsector) {
    FILE *f = fopen(output_file, "wb");
    if (!f) return false;

    fwrite(bin->code, 1, bin->size, f);

    if (bootsector) {
        // Pad up to 510 bytes, then write 0x55, 0xAA boot signature
        int cur = bin->size;
        while (cur < 510) {
            fputc(0x00, f);
            cur++;
        }
        fputc(0x55, f);
        fputc(0xAA, f);
    } else if (bin->rodata_size > 0) {
        // Append rodata
        fwrite(bin->rodata, 1, bin->rodata_size, f);
    }

    fclose(f);
    return true;
}

bool rawbin_write_bits01(const char *output_file, const NativeBinary *bin) {
    FILE *f = fopen(output_file, "w");
    if (!f) return false;

    fprintf(f, "# ==========================================================\n");
    fprintf(f, "# I Language - Pure Binary Machine Code Bitstream (01 Format)\n");
    fprintf(f, "# Total Code Bytes: %d  Total Bits: %d\n", bin->size, bin->size * 8);
    fprintf(f, "# ==========================================================\n\n");

    for (int i = 0; i < bin->size; i++) {
        uint8_t byte = bin->code[i];
        fprintf(f, "[0x%04X | bit %05d]: ", i, i * 8);
        for (int b = 7; b >= 0; b--) {
            int bit = (byte >> b) & 1;
            fputc(bit ? '1' : '0', f);
        }
        fprintf(f, "  # 0x%02X\n", byte);
    }

    if (bin->rodata_size > 0) {
        fprintf(f, "\n# ---- RODATA SEGMENT ----\n");
        for (int i = 0; i < bin->rodata_size; i++) {
            uint8_t byte = bin->rodata[i];
            fprintf(f, "[RO 0x%04X | bit %05d]: ", i, (bin->size + i) * 8);
            for (int b = 7; b >= 0; b--) {
                int bit = (byte >> b) & 1;
                fputc(bit ? '1' : '0', f);
            }
            fprintf(f, "  # 0x%02X '%c'\n", byte, (byte >= 32 && byte <= 126) ? byte : '.');
        }
    }

    fclose(f);
    return true;
}

bool rawbin_write_hex(const char *output_file, const NativeBinary *bin) {
    FILE *f = fopen(output_file, "w");
    if (!f) return false;

    fprintf(f, "# Intel HEX / Raw Byte Hexdump of I Compiled Binary\n");
    for (int i = 0; i < bin->size; i += 16) {
        fprintf(f, "%08X: ", i);
        for (int j = 0; j < 16; j++) {
            if (i + j < bin->size) {
                fprintf(f, "%02X ", bin->code[i + j]);
            } else {
                fprintf(f, "   ");
            }
        }
        fprintf(f, " |");
        for (int j = 0; j < 16 && (i + j < bin->size); j++) {
            uint8_t c = bin->code[i + j];
            fputc((c >= 32 && c <= 126) ? c : '.', f);
        }
        fprintf(f, "|\n");
    }
    fclose(f);
    return true;
}
