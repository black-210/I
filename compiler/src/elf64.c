#include "elf64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#pragma pack(push, 1)

typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

#pragma pack(pop)

bool elf64_write_executable(const char *output_file, const NativeBinary *bin) {
    FILE *f = fopen(output_file, "wb");
    if (!f) return false;

    // Layout in file:
    // 0x0000 - 0x003F : Elf64_Ehdr (64 bytes)
    // 0x0040 - 0x0077 : Elf64_Phdr Code (56 bytes)
    // 0x0078 - 0x00AF : Elf64_Phdr Rodata (56 bytes, if rodata present)
    int phnum = (bin->rodata_size > 0) ? 2 : 1;
    uint64_t headers_size = sizeof(Elf64_Ehdr) + (phnum * sizeof(Elf64_Phdr));
    // Pad code start to 16 bytes alignment after headers
    uint64_t code_file_offset = headers_size;
    if (code_file_offset % 16 != 0) {
        code_file_offset += (16 - (code_file_offset % 16));
    }

    uint64_t entry_vaddr = bin->vaddr_code + code_file_offset + bin->entry_offset;

    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    ehdr.e_ident[0] = 0x7F;
    ehdr.e_ident[1] = 'E';
    ehdr.e_ident[2] = 'L';
    ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2; // 64-bit
    ehdr.e_ident[5] = 1; // Little Endian
    ehdr.e_ident[6] = 1; // ELF Version
    ehdr.e_ident[7] = 0; // System V ABI
    ehdr.e_type = 2;     // ET_EXEC
    ehdr.e_machine = 0x3E; // x86-64
    ehdr.e_version = 1;
    ehdr.e_entry = entry_vaddr;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = phnum;

    // Code program header (RX)
    Elf64_Phdr ph_code;
    memset(&ph_code, 0, sizeof(ph_code));
    ph_code.p_type = 1; // PT_LOAD
    ph_code.p_flags = 5; // PF_R | PF_X
    ph_code.p_offset = 0;
    ph_code.p_vaddr = bin->vaddr_code;
    ph_code.p_paddr = bin->vaddr_code;
    ph_code.p_filesz = code_file_offset + bin->size;
    ph_code.p_memsz = code_file_offset + bin->size;
    ph_code.p_align = 0x1000;

    fwrite(&ehdr, 1, sizeof(ehdr), f);
    fwrite(&ph_code, 1, sizeof(ph_code), f);

    uint64_t rodata_file_offset = 0;
    if (phnum > 1) {
        rodata_file_offset = 0x1000; // Next 4KB page
        Elf64_Phdr ph_ro;
        memset(&ph_ro, 0, sizeof(ph_ro));
        ph_ro.p_type = 1; // PT_LOAD
        ph_ro.p_flags = 4; // PF_R
        ph_ro.p_offset = rodata_file_offset;
        ph_ro.p_vaddr = bin->vaddr_rodata;
        ph_ro.p_paddr = bin->vaddr_rodata;
        ph_ro.p_filesz = bin->rodata_size;
        ph_ro.p_memsz = bin->rodata_size;
        ph_ro.p_align = 0x1000;
        fwrite(&ph_ro, 1, sizeof(ph_ro), f);
    }

    // Pad to code_file_offset
    long cur = ftell(f);
    while (cur < (long)code_file_offset) {
        fputc(0x90, f); // NOP padding
        cur++;
    }

    // Write code
    fwrite(bin->code, 1, bin->size, f);

    // If rodata, pad to rodata_file_offset and write rodata
    if (phnum > 1) {
        cur = ftell(f);
        while (cur < (long)rodata_file_offset) {
            fputc(0, f);
            cur++;
        }
        fwrite(bin->rodata, 1, bin->rodata_size, f);
    }

    fclose(f);
    chmod(output_file, 0755); // make executable
    return true;
}
