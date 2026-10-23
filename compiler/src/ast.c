#include "ast.h"
#include <stdlib.h>
#include <string.h>

Expr *ast_new_expr(ExprKind kind, SourceLoc loc) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = kind;
    e->loc = loc;
    e->result_type.kind = TYPE_U64;
    e->result_type.bit_width = 64;
    return e;
}

Stmt *ast_new_stmt(StmtKind kind, SourceLoc loc) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = kind;
    s->loc = loc;
    return s;
}

DataType parse_type_name(const char *name) {
    DataType dt;
    memset(&dt, 0, sizeof(dt));
    strncpy(dt.name, name, sizeof(dt.name) - 1);

    if (strcmp(name, "u8") == 0 || strcmp(name, "byte") == 0 || strcmp(name, "bool") == 0) { dt.kind = TYPE_U8; dt.bit_width = 8; dt.is_signed = false; }
    else if (strcmp(name, "u16") == 0 || strcmp(name, "ushort") == 0) { dt.kind = TYPE_U16; dt.bit_width = 16; dt.is_signed = false; }
    else if (strcmp(name, "u32") == 0 || strcmp(name, "uint") == 0) { dt.kind = TYPE_U32; dt.bit_width = 32; dt.is_signed = false; }
    else if (strcmp(name, "u64") == 0 || strcmp(name, "ulong") == 0) { dt.kind = TYPE_U64; dt.bit_width = 64; dt.is_signed = false; }
    else if (strcmp(name, "i8") == 0)   { dt.kind = TYPE_I8; dt.bit_width = 8; dt.is_signed = true; }
    else if (strcmp(name, "i16") == 0 || strcmp(name, "short") == 0) { dt.kind = TYPE_I16; dt.bit_width = 16; dt.is_signed = true; }
    else if (strcmp(name, "i32") == 0 || strcmp(name, "int") == 0) { dt.kind = TYPE_I32; dt.bit_width = 32; dt.is_signed = true; }
    else if (strcmp(name, "i64") == 0 || strcmp(name, "long") == 0) { dt.kind = TYPE_I64; dt.bit_width = 64; dt.is_signed = true; }
    else if (strcmp(name, "b8") == 0)   { dt.kind = TYPE_B8; dt.bit_width = 8; dt.is_signed = false; }
    else if (strcmp(name, "b16") == 0) { dt.kind = TYPE_B16; dt.bit_width = 16; dt.is_signed = false; }
    else if (strcmp(name, "b32") == 0) { dt.kind = TYPE_B32; dt.bit_width = 32; dt.is_signed = false; }
    else if (strcmp(name, "b64") == 0) { dt.kind = TYPE_B64; dt.bit_width = 64; dt.is_signed = false; }
    else if (strcmp(name, "void") == 0) { dt.kind = TYPE_VOID; dt.bit_width = 0; }
    else if (strcmp(name, "raw") == 0 || strcmp(name, "*raw") == 0) { dt.kind = TYPE_RAW_PTR; dt.bit_width = 64; }
    else {
        dt.kind = TYPE_CUSTOM_LAYOUT;
        dt.bit_width = 64;
    }
    return dt;
}

const char *data_type_to_string(DataType t) {
    switch (t.kind) {
        case TYPE_VOID: return "void";
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_B8: return "b8";
        case TYPE_B16: return "b16";
        case TYPE_B32: return "b32";
        case TYPE_B64: return "b64";
        case TYPE_RAW_PTR: return "*raw";
        case TYPE_CUSTOM_LAYOUT: return "layout";
        default: return "unknown";
    }
}
