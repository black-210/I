#include "semantics.h"
#include "diagnostics.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    char name[64];
    DataType type;
    int stack_offset;
    bool is_mutable;
} Symbol;

typedef struct {
    Symbol symbols[128];
    int count;
    int current_stack_bytes;
} Scope;

static bool has_capability(AstProgram *prog, const char *cap_name) {
    for (int i = 0; i < prog->capability_count; i++) {
        if (strcmp(prog->capabilities[i], cap_name) == 0 ||
            strcmp(prog->capabilities[i], "all") == 0) {
            return true;
        }
    }
    return false;
}

static Symbol *find_symbol(Scope *scope, const char *name) {
    for (int i = scope->count - 1; i >= 0; i--) {
        if (strcmp(scope->symbols[i].name, name) == 0) {
            return &scope->symbols[i];
        }
    }
    return NULL;
}

static void analyze_expr(AstProgram *prog, Scope *scope, Expr *expr) {
    if (!expr) return;

    switch (expr->kind) {
        case EXPR_INT_LIT:
            break;
        case EXPR_STR_LIT:
            break;
        case EXPR_VAR: {
            Symbol *sym = find_symbol(scope, expr->as.var.name);
            if (!sym) {
                diag_report(SEV_ERROR, "S0001", expr->loc, "Undefined identifier '%s'", expr->as.var.name);
            } else {
                expr->as.var.local_offset = sym->stack_offset;
                expr->result_type = sym->type;
            }
            break;
        }
        case EXPR_BINARY:
            analyze_expr(prog, scope, expr->as.binary.left);
            analyze_expr(prog, scope, expr->as.binary.right);
            expr->result_type = expr->as.binary.left->result_type;
            break;
        case EXPR_UNARY:
            analyze_expr(prog, scope, expr->as.unary.operand);
            expr->result_type = expr->as.unary.operand->result_type;
            break;
        case EXPR_CALL:
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                analyze_expr(prog, scope, expr->as.call.args[i]);
            }
            break;
        case EXPR_INTRINSIC: {
            const char *iname = expr->as.intrinsic.name;
            for (int i = 0; i < expr->as.intrinsic.arg_count; i++) {
                analyze_expr(prog, scope, expr->as.intrinsic.args[i]);
            }

            // Capability checks
            if (strcmp(iname, "@out8") == 0 || strcmp(iname, "@in8") == 0 ||
                strcmp(iname, "@out16") == 0 || strcmp(iname, "@in16") == 0 ||
                strcmp(iname, "@out32") == 0 || strcmp(iname, "@in32") == 0 ||
                strcmp(iname, "@cli") == 0 || strcmp(iname, "@sti") == 0 ||
                strcmp(iname, "@hlt") == 0 || strcmp(iname, "@cpuid") == 0) {
                if (!has_capability(prog, "hardware_io") && !has_capability(prog, "kernel_ring0")) {
                    diag_report(SEV_ERROR, "S0002", expr->loc,
                        "Hardware intrinsic '%s' requires capability 'hardware_io' or 'kernel_ring0'", iname);
                    diag_note("Add 'require cap: hardware_io;' at the top of your system unit.");
                }
            } else if (strcmp(iname, "@load8") == 0 || strcmp(iname, "@store8") == 0 ||
                       strcmp(iname, "@load16") == 0 || strcmp(iname, "@store16") == 0 ||
                       strcmp(iname, "@load32") == 0 || strcmp(iname, "@store32") == 0 ||
                       strcmp(iname, "@load64") == 0 || strcmp(iname, "@store64") == 0) {
                if (!has_capability(prog, "raw_memory") && !has_capability(prog, "kernel_ring0")) {
                    diag_report(SEV_ERROR, "S0004", expr->loc,
                        "Direct memory intrinsic '%s' requires capability 'raw_memory' or 'kernel_ring0'", iname);
                    diag_note("Add 'require cap: raw_memory;' to declare raw memory access privilege.");
                }
            } else if (strcmp(iname, "@syscall") == 0) {
                if (!has_capability(prog, "host_syscall") && !has_capability(prog, "systems")) {
                    diag_report(SEV_ERROR, "S0003", expr->loc,
                        "Direct syscall '%s' requires capability 'host_syscall'", iname);
                    diag_note("Add 'require cap: host_syscall;' to declare syscall privilege.");
                }
            }
            break;
        }
        case EXPR_FIELD_ACCESS:
            analyze_expr(prog, scope, expr->as.field.target);
            break;
    }
}

static uint64_t eval_const_expr(Expr *e, bool *ok) {
    if (!e) { *ok = false; return 0; }
    if (e->kind == EXPR_INT_LIT) {
        *ok = true;
        return e->as.int_lit.val;
    }
    if (e->kind == EXPR_BINARY) {
        bool lok = false, rok = false;
        uint64_t l = eval_const_expr(e->as.binary.left, &lok);
        uint64_t r = eval_const_expr(e->as.binary.right, &rok);
        if (lok && rok) {
            *ok = true;
            switch (e->as.binary.op) {
                case TOK_OP_ADD:
                case TOK_OP_ADD_WRAP:
                case TOK_OP_ADD_CHECKED: return l + r;
                case TOK_OP_SUB:
                case TOK_OP_SUB_WRAP:
                case TOK_OP_SUB_CHECKED: return l - r;
                case TOK_OP_MUL:
                case TOK_OP_MUL_WRAP:
                case TOK_OP_MUL_CHECKED: return l * r;
                case TOK_OP_AND: return l & r;
                case TOK_OP_OR: return l | r;
                case TOK_OP_XOR: return l ^ r;
                case TOK_OP_SHL: return l << r;
                case TOK_OP_SHR: return l >> r;
                case TOK_OP_EQ: return (l == r) ? 1 : 0;
                case TOK_OP_NE: return (l != r) ? 1 : 0;
                case TOK_OP_LT: return (l < r) ? 1 : 0;
                case TOK_OP_LE: return (l <= r) ? 1 : 0;
                case TOK_OP_GT: return (l > r) ? 1 : 0;
                case TOK_OP_GE: return (l >= r) ? 1 : 0;
                default: break;
            }
        }
    }
    *ok = false;
    return 0;
}

static void analyze_stmt(AstProgram *prog, Scope *scope, Stmt *stmt) {
    if (!stmt) return;

    switch (stmt->kind) {
        case STMT_BIND:
        case STMT_MUT: {
            analyze_expr(prog, scope, stmt->as.decl.init);
            scope->current_stack_bytes += 8;
            int offset = -scope->current_stack_bytes;
            stmt->as.decl.stack_offset = offset;

            Symbol sym;
            strncpy(sym.name, stmt->as.decl.name, sizeof(sym.name) - 1);
            sym.type = stmt->as.decl.type;
            sym.stack_offset = offset;
            sym.is_mutable = stmt->as.decl.is_mutable;
            scope->symbols[scope->count++] = sym;
            break;
        }
        case STMT_ASSIGN: {
            Symbol *sym = find_symbol(scope, stmt->as.assign.name);
            if (!sym) {
                diag_report(SEV_ERROR, "S0005", stmt->loc, "Cannot assign to undeclared variable '%s'", stmt->as.assign.name);
            } else if (!sym->is_mutable) {
                diag_report(SEV_ERROR, "S0006", stmt->loc, "Cannot mutate immutable binding '%s'", stmt->as.assign.name);
                diag_note("Variables declared with 'bind' are strictly immutable. Use 'mut' to allow mutation.");
            } else {
                stmt->as.assign.stack_offset = sym->stack_offset;
            }
            analyze_expr(prog, scope, stmt->as.assign.val);
            break;
        }
        case STMT_BRANCH:
            analyze_expr(prog, scope, stmt->as.branch.cond);
            for (int i = 0; i < stmt->as.branch.then_block.count; i++) {
                analyze_stmt(prog, scope, stmt->as.branch.then_block.stmts[i]);
            }
            if (stmt->as.branch.has_else) {
                for (int i = 0; i < stmt->as.branch.else_block.count; i++) {
                    analyze_stmt(prog, scope, stmt->as.branch.else_block.stmts[i]);
                }
            }
            break;
        case STMT_CYCLE:
            analyze_stmt(prog, scope, stmt->as.cycle.init);
            analyze_expr(prog, scope, stmt->as.cycle.cond);
            for (int i = 0; i < stmt->as.cycle.body.count; i++) {
                analyze_stmt(prog, scope, stmt->as.cycle.body.stmts[i]);
            }
            analyze_stmt(prog, scope, stmt->as.cycle.step);
            break;
        case STMT_REPEAT:
            for (int i = 0; i < stmt->as.repeat_until.body.count; i++) {
                analyze_stmt(prog, scope, stmt->as.repeat_until.body.stmts[i]);
            }
            analyze_expr(prog, scope, stmt->as.repeat_until.cond);
            break;
        case STMT_YIELD:
            if (stmt->as.yield_stmt.val) {
                analyze_expr(prog, scope, stmt->as.yield_stmt.val);
            }
            break;
        case STMT_HALT:
            break;
        case STMT_TRAP:
            break;
        case STMT_VERIFY: {
            bool ok = false;
            uint64_t res = eval_const_expr(stmt->as.verify_stmt.expr, &ok);
            if (!ok) {
                diag_report(SEV_ERROR, "S0007", stmt->loc, "Expression in 'verify' must be compile-time evaluable");
            } else if (res == 0) {
                diag_report(SEV_ERROR, "S0008", stmt->loc, "Static verification contract failed: assertion is FALSE");
            }
            break;
        }
        case STMT_EXPR:
            analyze_expr(prog, scope, stmt->as.expr_stmt.expr);
            break;
    }
}

bool semantics_analyze(AstProgram *prog) {
    for (int r = 0; r < prog->routine_count; r++) {
        Routine *rt = &prog->routines[r];
        Scope scope = {0};

        // Parameters setup
        for (int p = 0; p < rt->param_count; p++) {
            scope.current_stack_bytes += 8;
            Symbol sym;
            strncpy(sym.name, rt->params[p].name, sizeof(sym.name) - 1);
            sym.type = rt->params[p].type;
            sym.stack_offset = -scope.current_stack_bytes;
            sym.is_mutable = false;
            scope.symbols[scope.count++] = sym;
        }

        for (int i = 0; i < rt->body.count; i++) {
            analyze_stmt(prog, &scope, rt->body.stmts[i]);
        }
        // Round up stack frame to 16-byte alignment
        int total = scope.current_stack_bytes;
        if (total % 16 != 0) total += (16 - (total % 16));
        rt->total_stack_bytes = total;
    }

    return !diag_has_errors();
}
