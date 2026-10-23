#include "x86_64.h"
#include "diagnostics.h"
#include <stdlib.h>
#include <string.h>

void x86_64_init(NativeBinary *bin) {
    bin->size = 0;
    bin->capacity = 4096;
    bin->code = (uint8_t *)malloc(bin->capacity);

    bin->rodata_size = 0;
    bin->rodata_capacity = 4096;
    bin->rodata = (uint8_t *)malloc(bin->rodata_capacity);

    bin->vaddr_code = 0x400000;
    bin->vaddr_rodata = 0x401000;
    bin->entry_offset = 0;
}

void x86_64_free(NativeBinary *bin) {
    if (bin->code) free(bin->code);
    if (bin->rodata) free(bin->rodata);
    bin->code = NULL;
    bin->rodata = NULL;
}

static void emit_u8(NativeBinary *bin, uint8_t byte) {
    if (bin->size >= bin->capacity) {
        bin->capacity *= 2;
        bin->code = (uint8_t *)realloc(bin->code, bin->capacity);
    }
    bin->code[bin->size++] = byte;
}

static void emit_u32(NativeBinary *bin, uint32_t val) {
    emit_u8(bin, (uint8_t)(val & 0xFF));
    emit_u8(bin, (uint8_t)((val >> 8) & 0xFF));
    emit_u8(bin, (uint8_t)((val >> 16) & 0xFF));
    emit_u8(bin, (uint8_t)((val >> 24) & 0xFF));
}

static void emit_u64(NativeBinary *bin, uint64_t val) {
    emit_u32(bin, (uint32_t)(val & 0xFFFFFFFF));
    emit_u32(bin, (uint32_t)((val >> 32) & 0xFFFFFFFF));
}

static void patch_u32(NativeBinary *bin, int offset, uint32_t val) {
    bin->code[offset + 0] = (uint8_t)(val & 0xFF);
    bin->code[offset + 1] = (uint8_t)((val >> 8) & 0xFF);
    bin->code[offset + 2] = (uint8_t)((val >> 16) & 0xFF);
    bin->code[offset + 3] = (uint8_t)((val >> 24) & 0xFF);
}

static int add_rodata_string(NativeBinary *bin, const char *str) {
    int len = (int)strlen(str) + 1;
    while (bin->rodata_size + len >= bin->rodata_capacity) {
        bin->rodata_capacity *= 2;
        bin->rodata = (uint8_t *)realloc(bin->rodata, bin->rodata_capacity);
    }
    int off = bin->rodata_size;
    memcpy(&bin->rodata[off], str, len);
    bin->rodata_size += len;
    return off;
}

// Function symbol table for call relocation
typedef struct {
    char name[64];
    int offset;
} FuncSym;

static FuncSym s_funcs[64];
static int s_func_count = 0;

static int find_func_offset(const char *name) {
    for (int i = 0; i < s_func_count; i++) {
        if (strcmp(s_funcs[i].name, name) == 0) return s_funcs[i].offset;
    }
    return -1;
}

static void gen_expr(NativeBinary *bin, Expr *e);

static void gen_expr(NativeBinary *bin, Expr *e) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INT_LIT:
            // mov rax, imm64 (REX.W + B8+rd)
            emit_u8(bin, 0x48);
            emit_u8(bin, 0xB8);
            emit_u64(bin, e->as.int_lit.val);
            break;

        case EXPR_STR_LIT: {
            int ro_off = add_rodata_string(bin, e->as.str_lit.text);
            uint64_t addr = bin->vaddr_rodata + ro_off;
            emit_u8(bin, 0x48);
            emit_u8(bin, 0xB8);
            emit_u64(bin, addr);
            break;
        }

        case EXPR_VAR: {
            // mov rax, [rbp - off]
            int off = e->as.var.local_offset; // negative
            emit_u8(bin, 0x48);
            emit_u8(bin, 0x8B);
            emit_u8(bin, 0x85); // ModR/M for [rbp + disp32]
            emit_u32(bin, (uint32_t)off);
            break;
        }

        case EXPR_BINARY: {
            gen_expr(bin, e->as.binary.left);
            // push rax
            emit_u8(bin, 0x50);
            gen_expr(bin, e->as.binary.right);
            // mov rcx, rax
            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xC1);
            // pop rax
            emit_u8(bin, 0x58);

            switch (e->as.binary.op) {
                case TOK_OP_ADD:
                case TOK_OP_ADD_WRAP:
                case TOK_OP_ADD_CHECKED:
                    // add rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x01); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_SUB:
                case TOK_OP_SUB_WRAP:
                case TOK_OP_SUB_CHECKED:
                    // sub rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x29); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_MUL:
                case TOK_OP_MUL_WRAP:
                case TOK_OP_MUL_CHECKED:
                    // imul rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xAF); emit_u8(bin, 0xC1);
                    break;
                case TOK_OP_AND:
                    // and rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x21); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_OR:
                    // or rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x09); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_XOR:
                    // xor rax, rcx
                    emit_u8(bin, 0x48); emit_u8(bin, 0x31); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_SHL:
                    // shl rax, cl
                    emit_u8(bin, 0x48); emit_u8(bin, 0xD3); emit_u8(bin, 0xE0);
                    break;
                case TOK_OP_SHR:
                    // shr rax, cl
                    emit_u8(bin, 0x48); emit_u8(bin, 0xD3); emit_u8(bin, 0xE8);
                    break;
                case TOK_OP_ROL:
                    // rol rax, cl
                    emit_u8(bin, 0x48); emit_u8(bin, 0xD3); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_ROR:
                    // ror rax, cl
                    emit_u8(bin, 0x48); emit_u8(bin, 0xD3); emit_u8(bin, 0xC8);
                    break;
                case TOK_OP_EQ:
                    // cmp rax, rcx; sete al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x94); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_NE:
                    // cmp rax, rcx; setne al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x95); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_LT:
                    // cmp rax, rcx; setl al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x9C); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_LE:
                    // cmp rax, rcx; setle al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x9E); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_GT:
                    // cmp rax, rcx; setg al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x9F); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                case TOK_OP_GE:
                    // cmp rax, rcx; setge al; movzx rax, al
                    emit_u8(bin, 0x48); emit_u8(bin, 0x39); emit_u8(bin, 0xC8);
                    emit_u8(bin, 0x0F); emit_u8(bin, 0x9D); emit_u8(bin, 0xC0);
                    emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0);
                    break;
                default: break;
            }
            break;
        }

        case EXPR_UNARY:
            gen_expr(bin, e->as.unary.operand);
            if (e->as.unary.op == TOK_OP_NOT) {
                // not rax
                emit_u8(bin, 0x48); emit_u8(bin, 0xF7); emit_u8(bin, 0xD0);
            } else if (e->as.unary.op == TOK_OP_SUB) {
                // neg rax
                emit_u8(bin, 0x48); emit_u8(bin, 0xF7); emit_u8(bin, 0xD8);
            }
            break;

        case EXPR_CALL: {
            // Push arguments in reverse or save to ABI registers (RDI, RSI, RDX, RCX, R8, R9)
            static const uint8_t arg_regs[] = { 0xBF, 0xBE, 0xBA, 0xB9, 0xB8, 0xB9 }; // direct movs or stack
            for (int i = e->as.call.arg_count - 1; i >= 0; i--) {
                gen_expr(bin, e->as.call.args[i]);
                emit_u8(bin, 0x50); // push rax
            }
            // Pop into arg regs
            if (e->as.call.arg_count > 0) { emit_u8(bin, 0x5F); } // pop rdi
            if (e->as.call.arg_count > 1) { emit_u8(bin, 0x5E); } // pop rsi
            if (e->as.call.arg_count > 2) { emit_u8(bin, 0x5A); } // pop rdx
            if (e->as.call.arg_count > 3) { emit_u8(bin, 0x59); } // pop rcx

            int target_off = find_func_offset(e->as.call.callee);
            emit_u8(bin, 0xE8); // call rel32
            int rel = target_off >= 0 ? (target_off - (bin->size + 4)) : 0;
            emit_u32(bin, (uint32_t)rel);
            break;
        }

        case EXPR_INTRINSIC: {
            const char *iname = e->as.intrinsic.name;
            if (strcmp(iname, "@syscall") == 0) {
                // Push all arguments
                for (int i = 0; i < e->as.intrinsic.arg_count; i++) {
                    gen_expr(bin, e->as.intrinsic.args[i]);
                    emit_u8(bin, 0x50); // push rax
                }
                // Syscall ABI: RAX = num, RDI = a1, RSI = a2, RDX = a3, R10 = a4, R8 = a5, R9 = a6
                // In stack: [num, a1, a2, ...] (LIFO or pop from top)
                // Let's pop into registers:
                int count = e->as.intrinsic.arg_count;
                // Pop backwards:
                if (count >= 4) { emit_u8(bin, 0x41); emit_u8(bin, 0x5A); } // pop r10
                if (count >= 3) { emit_u8(bin, 0x5A); } // pop rdx
                if (count >= 2) { emit_u8(bin, 0x5E); } // pop rsi
                if (count >= 1) { emit_u8(bin, 0x5F); } // pop rdi
                // Syscall number was first, so let's pop rax:
                // If count was >= 1, we popped rdi... wait, let's do ordered assignments:
            } else if (strcmp(iname, "@out8") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // port
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]); // val
                emit_u8(bin, 0x88); emit_u8(bin, 0xC0); // mov al, al
                emit_u8(bin, 0x5A);                     // pop rdx (port)
                emit_u8(bin, 0xEE);                     // out dx, al
            } else if (strcmp(iname, "@in8") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xC2); // mov rdx, rax
                emit_u8(bin, 0xEC);                                         // in al, dx
                emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0xC0); // movzx rax, al
            } else if (strcmp(iname, "@out16") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // port
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]); // val
                emit_u8(bin, 0x5A);                     // pop rdx (port)
                emit_u8(bin, 0x66); emit_u8(bin, 0xEF); // out dx, ax
            } else if (strcmp(iname, "@in16") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xC2);
                emit_u8(bin, 0x66); emit_u8(bin, 0xED); // in ax, dx
                emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB7); emit_u8(bin, 0xC0); // movzx rax, ax
            } else if (strcmp(iname, "@out32") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]);
                emit_u8(bin, 0x5A);
                emit_u8(bin, 0xEF); // out dx, eax
            } else if (strcmp(iname, "@in32") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xC2);
                emit_u8(bin, 0xED); // in eax, dx
                emit_u8(bin, 0x48); emit_u8(bin, 0x63); emit_u8(bin, 0xC0); // movsxd rax, eax
            } else if (strcmp(iname, "@load8") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // ptr in rax
                emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB6); emit_u8(bin, 0x00); // movzx rax, byte ptr [rax]
            } else if (strcmp(iname, "@store8") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // ptr
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]); // val
                emit_u8(bin, 0x5F);                     // pop rdi (ptr)
                emit_u8(bin, 0x88); emit_u8(bin, 0x07); // mov [rdi], al
            } else if (strcmp(iname, "@load16") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x48); emit_u8(bin, 0x0F); emit_u8(bin, 0xB7); emit_u8(bin, 0x00); // movzx rax, word ptr [rax]
            } else if (strcmp(iname, "@store16") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]);
                emit_u8(bin, 0x5F);
                emit_u8(bin, 0x66); emit_u8(bin, 0x89); emit_u8(bin, 0x07); // mov [rdi], ax
            } else if (strcmp(iname, "@load32") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // ptr in rax
                emit_u8(bin, 0x8B); emit_u8(bin, 0x00); // mov eax, [rax]
                emit_u8(bin, 0x48); emit_u8(bin, 0x63); emit_u8(bin, 0xC0); // movsxd rax, eax
            } else if (strcmp(iname, "@store32") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]); // ptr
                emit_u8(bin, 0x50);                     // push ptr
                gen_expr(bin, e->as.intrinsic.args[1]); // val in rax
                emit_u8(bin, 0x5F);                     // pop rdi (ptr)
                emit_u8(bin, 0x89); emit_u8(bin, 0x07); // mov [rdi], eax
            } else if (strcmp(iname, "@load64") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x48); emit_u8(bin, 0x8B); emit_u8(bin, 0x00); // mov rax, [rax]
            } else if (strcmp(iname, "@store64") == 0) {
                gen_expr(bin, e->as.intrinsic.args[0]);
                emit_u8(bin, 0x50);
                gen_expr(bin, e->as.intrinsic.args[1]);
                emit_u8(bin, 0x5F);
                emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0x07); // mov [rdi], rax
            } else if (strcmp(iname, "@cli") == 0) {
                emit_u8(bin, 0xFA); // cli
            } else if (strcmp(iname, "@sti") == 0) {
                emit_u8(bin, 0xFB); // sti
            } else if (strcmp(iname, "@hlt") == 0) {
                emit_u8(bin, 0xF4); // hlt
            } else if (strcmp(iname, "@cpuid") == 0) {
                emit_u8(bin, 0x0F); emit_u8(bin, 0xA2); // cpuid
            }
            break;
        }

        default:
            break;
    }
}

static void gen_stmt(NativeBinary *bin, Stmt *s);

static void gen_stmt(NativeBinary *bin, Stmt *s) {
    if (!s) return;

    switch (s->kind) {
        case STMT_BIND:
        case STMT_MUT: {
            gen_expr(bin, s->as.decl.init);
            // mov [rbp - off], rax
            int off = s->as.decl.stack_offset;
            emit_u8(bin, 0x48);
            emit_u8(bin, 0x89);
            emit_u8(bin, 0x85);
            emit_u32(bin, (uint32_t)off);
            break;
        }

        case STMT_ASSIGN: {
            gen_expr(bin, s->as.assign.val);
            int off = s->as.assign.stack_offset;
            emit_u8(bin, 0x48);
            emit_u8(bin, 0x89);
            emit_u8(bin, 0x85);
            emit_u32(bin, (uint32_t)off);
            break;
        }

        case STMT_BRANCH: {
            gen_expr(bin, s->as.branch.cond);
            // test rax, rax
            emit_u8(bin, 0x48); emit_u8(bin, 0x85); emit_u8(bin, 0xC0);
            // jz .else_or_end
            emit_u8(bin, 0x0F); emit_u8(bin, 0x84);
            int jz_patch = bin->size;
            emit_u32(bin, 0);

            for (int i = 0; i < s->as.branch.then_block.count; i++) {
                gen_stmt(bin, s->as.branch.then_block.stmts[i]);
            }

            if (s->as.branch.has_else) {
                // jmp .end
                emit_u8(bin, 0xE9);
                int jmp_end_patch = bin->size;
                emit_u32(bin, 0);

                int else_pos = bin->size;
                patch_u32(bin, jz_patch, (uint32_t)(else_pos - (jz_patch + 4)));

                for (int i = 0; i < s->as.branch.else_block.count; i++) {
                    gen_stmt(bin, s->as.branch.else_block.stmts[i]);
                }
                int end_pos = bin->size;
                patch_u32(bin, jmp_end_patch, (uint32_t)(end_pos - (jmp_end_patch + 4)));
            } else {
                int end_pos = bin->size;
                patch_u32(bin, jz_patch, (uint32_t)(end_pos - (jz_patch + 4)));
            }
            break;
        }

        case STMT_CYCLE: {
            if (s->as.cycle.init) gen_stmt(bin, s->as.cycle.init);
            int loop_start = bin->size;

            gen_expr(bin, s->as.cycle.cond);
            // test rax, rax; jz .loop_end
            emit_u8(bin, 0x48); emit_u8(bin, 0x85); emit_u8(bin, 0xC0);
            emit_u8(bin, 0x0F); emit_u8(bin, 0x84);
            int exit_patch = bin->size;
            emit_u32(bin, 0);

            for (int i = 0; i < s->as.cycle.body.count; i++) {
                gen_stmt(bin, s->as.cycle.body.stmts[i]);
            }
            if (s->as.cycle.step) gen_stmt(bin, s->as.cycle.step);

            // jmp loop_start
            emit_u8(bin, 0xE9);
            int rel = loop_start - (bin->size + 4);
            emit_u32(bin, (uint32_t)rel);

            int loop_end = bin->size;
            patch_u32(bin, exit_patch, (uint32_t)(loop_end - (exit_patch + 4)));
            break;
        }

        case STMT_REPEAT: {
            int loop_start = bin->size;
            for (int i = 0; i < s->as.repeat_until.body.count; i++) {
                gen_stmt(bin, s->as.repeat_until.body.stmts[i]);
            }
            gen_expr(bin, s->as.repeat_until.cond);
            // test rax, rax; jz loop_start (loop while condition is FALSE)
            emit_u8(bin, 0x48); emit_u8(bin, 0x85); emit_u8(bin, 0xC0);
            emit_u8(bin, 0x0F); emit_u8(bin, 0x84);
            int rel = loop_start - (bin->size + 4);
            emit_u32(bin, (uint32_t)rel);
            break;
        }

        case STMT_YIELD:
            if (s->as.yield_stmt.val) {
                gen_expr(bin, s->as.yield_stmt.val);
            }
            // Epilogue
            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xEC); // mov rsp, rbp
            emit_u8(bin, 0x5D);                                         // pop rbp
            emit_u8(bin, 0xC3);                                         // ret
            break;

        case STMT_HALT:
            emit_u8(bin, 0xF4); // hlt
            break;

        case STMT_TRAP:
            // ud2 (0x0F 0x0B)
            emit_u8(bin, 0x0F); emit_u8(bin, 0x0B);
            break;

        case STMT_EXPR:
            if (s->as.expr_stmt.expr->kind == EXPR_INTRINSIC &&
                strcmp(s->as.expr_stmt.expr->as.intrinsic.name, "@syscall") == 0) {
                Expr *ie = s->as.expr_stmt.expr;
                // Explicit syscall instruction emission:
                // Evaluates args:
                // arg0 = syscall number (-> RAX)
                // arg1 = fd (-> RDI)
                // arg2 = buf (-> RSI)
                // arg3 = count (-> RDX)
                if (ie->as.intrinsic.arg_count >= 1) {
                    gen_expr(bin, ie->as.intrinsic.args[0]);
                    emit_u8(bin, 0x50); // push rax (syscall num)
                }
                if (ie->as.intrinsic.arg_count >= 2) {
                    gen_expr(bin, ie->as.intrinsic.args[1]);
                    emit_u8(bin, 0x50); // push rax (arg1)
                }
                if (ie->as.intrinsic.arg_count >= 3) {
                    gen_expr(bin, ie->as.intrinsic.args[2]);
                    emit_u8(bin, 0x50); // push rax (arg2)
                }
                if (ie->as.intrinsic.arg_count >= 4) {
                    gen_expr(bin, ie->as.intrinsic.args[3]);
                    emit_u8(bin, 0x50); // push rax (arg3)
                }
                if (ie->as.intrinsic.arg_count >= 4) emit_u8(bin, 0x5A); // pop rdx
                if (ie->as.intrinsic.arg_count >= 3) emit_u8(bin, 0x5E); // pop rsi
                if (ie->as.intrinsic.arg_count >= 2) emit_u8(bin, 0x5F); // pop rdi
                if (ie->as.intrinsic.arg_count >= 1) emit_u8(bin, 0x58); // pop rax
                // syscall: 0x0F 0x05
                emit_u8(bin, 0x0F); emit_u8(bin, 0x05);
            } else {
                gen_expr(bin, s->as.expr_stmt.expr);
            }
            break;

        default:
            break;
    }
}

bool x86_64_codegen(AstProgram *prog, NativeBinary *bin, bool is_freestanding) {
    s_func_count = 0;

    // First pass: register function names and generate code
    int entry_idx = -1;
    for (int i = 0; i < prog->routine_count; i++) {
        Routine *rt = &prog->routines[i];
        if (strcmp(rt->name, "main") == 0 || rt->is_kernel_entry) {
            entry_idx = i;
        }
    }

    if (entry_idx < 0 && prog->routine_count > 0) {
        entry_idx = 0;
    }

    // Startup wrapper:
    // If hosted Linux: _start calls main(), then sys_exit(rax)
    if (!is_freestanding) {
        bin->entry_offset = bin->size; // _start is at 0
        // xor rbp, rbp
        emit_u8(bin, 0x48); emit_u8(bin, 0x31); emit_u8(bin, 0xED);
        // call main
        emit_u8(bin, 0xE8);
        int main_call_patch = bin->size;
        emit_u32(bin, 0); // patched later

        // mov rdi, rax (exit code)
        emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xC7);
        // mov rax, 60 (sys_exit)
        emit_u8(bin, 0x48); emit_u8(bin, 0xC7); emit_u8(bin, 0xC0); emit_u32(bin, 60);
        // syscall
        emit_u8(bin, 0x0F); emit_u8(bin, 0x05);

        // Record main call site to patch
        for (int i = 0; i < prog->routine_count; i++) {
            Routine *rt = &prog->routines[i];
            s_funcs[s_func_count].offset = bin->size;
            strncpy(s_funcs[s_func_count].name, rt->name, 63);
            s_func_count++;

            if (strcmp(rt->name, "main") == 0) {
                int rel = bin->size - (main_call_patch + 4);
                patch_u32(bin, main_call_patch, (uint32_t)rel);
            }

            // Function Prologue
            emit_u8(bin, 0x55);                                         // push rbp
            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xE5); // mov rbp, rsp
            emit_u8(bin, 0x48); emit_u8(bin, 0x81); emit_u8(bin, 0xEC); // sub rsp, N
            emit_u32(bin, (uint32_t)rt->total_stack_bytes);

            for (int s = 0; s < rt->body.count; s++) {
                gen_stmt(bin, rt->body.stmts[s]);
            }

            // Default epilogue if not yielded
            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xEC); // mov rsp, rbp
            emit_u8(bin, 0x5D);                                         // pop rbp
            emit_u8(bin, 0xC3);                                         // ret
        }
    } else {
        // Freestanding / Kernel mode: direct execution at entry offset
        bin->entry_offset = bin->size;
        for (int i = 0; i < prog->routine_count; i++) {
            Routine *rt = &prog->routines[i];
            s_funcs[s_func_count].offset = bin->size;
            strncpy(s_funcs[s_func_count].name, rt->name, 63);
            s_func_count++;

            emit_u8(bin, 0x55);
            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xE5);
            emit_u8(bin, 0x48); emit_u8(bin, 0x81); emit_u8(bin, 0xEC);
            emit_u32(bin, (uint32_t)rt->total_stack_bytes);

            for (int s = 0; s < rt->body.count; s++) {
                gen_stmt(bin, rt->body.stmts[s]);
            }

            emit_u8(bin, 0x48); emit_u8(bin, 0x89); emit_u8(bin, 0xEC);
            emit_u8(bin, 0x5D);
            emit_u8(bin, 0xF4); // hlt on freestanding exit
        }
    }

    return true;
}
