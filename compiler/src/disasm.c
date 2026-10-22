#include "disasm.h"

void disasm_print(FILE *out, const NativeBinary *bin) {
    fprintf(out, "; ==========================================================\n");
    fprintf(out, "; I Disassembler - Native x86-64 Machine Instructions\n");
    fprintf(out, "; Code Size: %d bytes (Entry offset: +0x%X)\n", bin->size, bin->entry_offset);
    fprintf(out, "; ==========================================================\n\n");

    int i = 0;
    while (i < bin->size) {
        int start = i;
        uint8_t b0 = bin->code[i++];
        char text[128] = {0};

        if (b0 == 0x55) {
            snprintf(text, sizeof(text), "push rbp");
        } else if (b0 == 0x5D) {
            snprintf(text, sizeof(text), "pop rbp");
        } else if (b0 == 0xC3) {
            snprintf(text, sizeof(text), "ret");
        } else if (b0 == 0xF4) {
            snprintf(text, sizeof(text), "hlt");
        } else if (b0 == 0xFA) {
            snprintf(text, sizeof(text), "cli");
        } else if (b0 == 0xFB) {
            snprintf(text, sizeof(text), "sti");
        } else if (b0 == 0x0F && i < bin->size && bin->code[i] == 0x05) {
            i++;
            snprintf(text, sizeof(text), "syscall");
        } else if (b0 == 0x0F && i < bin->size && bin->code[i] == 0x0B) {
            i++;
            snprintf(text, sizeof(text), "ud2 (trap)");
        } else if (b0 == 0xE8 && i + 4 <= bin->size) {
            int32_t rel = (int32_t)(bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24));
            i += 4;
            snprintf(text, sizeof(text), "call 0x%04X (rel %d)", i + rel, rel);
        } else if (b0 == 0xE9 && i + 4 <= bin->size) {
            int32_t rel = (int32_t)(bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24));
            i += 4;
            snprintf(text, sizeof(text), "jmp 0x%04X (rel %d)", i + rel, rel);
        } else if (b0 == 0x0F && i < bin->size && bin->code[i] == 0x84 && i + 5 <= bin->size) {
            i++; // skip 0x84
            int32_t rel = (int32_t)(bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24));
            i += 4;
            snprintf(text, sizeof(text), "jz 0x%04X (rel %d)", i + rel, rel);
        } else if (b0 == 0x48 && i < bin->size) {
            uint8_t b1 = bin->code[i++];
            if (b1 == 0x89 && i < bin->size && bin->code[i] == 0xE5) {
                i++;
                snprintf(text, sizeof(text), "mov rbp, rsp");
            } else if (b1 == 0x89 && i < bin->size && bin->code[i] == 0xEC) {
                i++;
                snprintf(text, sizeof(text), "mov rsp, rbp");
            } else if (b1 == 0x89 && i < bin->size && bin->code[i] == 0xC1) {
                i++;
                snprintf(text, sizeof(text), "mov rcx, rax");
            } else if (b1 == 0x89 && i < bin->size && bin->code[i] == 0xC7) {
                i++;
                snprintf(text, sizeof(text), "mov rdi, rax");
            } else if (b1 == 0x31 && i < bin->size && bin->code[i] == 0xED) {
                i++;
                snprintf(text, sizeof(text), "xor rbp, rbp");
            } else if (b1 == 0xB8 && i + 8 <= bin->size) {
                uint64_t val = 0;
                for (int k = 0; k < 8; k++) val |= ((uint64_t)bin->code[i + k]) << (k * 8);
                i += 8;
                snprintf(text, sizeof(text), "mov rax, 0x%lX", val);
            } else if (b1 == 0xC7 && i < bin->size && bin->code[i] == 0xC0 && i + 5 <= bin->size) {
                i++;
                uint32_t val = bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24);
                i += 4;
                snprintf(text, sizeof(text), "mov rax, %u", val);
            } else if (b1 == 0x81 && i < bin->size && bin->code[i] == 0xEC && i + 5 <= bin->size) {
                i++;
                uint32_t val = bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24);
                i += 4;
                snprintf(text, sizeof(text), "sub rsp, %u", val);
            } else if (b1 == 0x89 && i < bin->size && bin->code[i] == 0x85 && i + 5 <= bin->size) {
                i++;
                int32_t off = (int32_t)(bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24));
                i += 4;
                snprintf(text, sizeof(text), "mov [rbp %d], rax", off);
            } else if (b1 == 0x8B && i < bin->size && bin->code[i] == 0x85 && i + 5 <= bin->size) {
                i++;
                int32_t off = (int32_t)(bin->code[i] | (bin->code[i+1] << 8) | (bin->code[i+2] << 16) | (bin->code[i+3] << 24));
                i += 4;
                snprintf(text, sizeof(text), "mov rax, [rbp %d]", off);
            } else if (b1 == 0x01 && i < bin->size && bin->code[i] == 0xC8) {
                i++;
                snprintf(text, sizeof(text), "add rax, rcx");
            } else if (b1 == 0x29 && i < bin->size && bin->code[i] == 0xC8) {
                i++;
                snprintf(text, sizeof(text), "sub rax, rcx");
            } else if (b1 == 0x21 && i < bin->size && bin->code[i] == 0xC8) {
                i++;
                snprintf(text, sizeof(text), "and rax, rcx");
            } else if (b1 == 0x09 && i < bin->size && bin->code[i] == 0xC8) {
                i++;
                snprintf(text, sizeof(text), "or rax, rcx");
            } else if (b1 == 0x31 && i < bin->size && bin->code[i] == 0xC8) {
                i++;
                snprintf(text, sizeof(text), "xor rax, rcx");
            } else if (b1 == 0x85 && i < bin->size && bin->code[i] == 0xC0) {
                i++;
                snprintf(text, sizeof(text), "test rax, rax");
            } else {
                snprintf(text, sizeof(text), "rex.w %02X", b1);
            }
        } else {
            snprintf(text, sizeof(text), "db 0x%02X", b0);
        }

        fprintf(out, "0x%04X:  ", start);
        for (int k = start; k < i; k++) fprintf(out, "%02X ", bin->code[k]);
        for (int k = (i - start); k < 7; k++) fprintf(out, "   ");
        fprintf(out, " %s\n", text);
    }
}
