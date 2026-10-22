#ifndef I_AST_H
#define I_AST_H

#include "lexer.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TYPE_VOID,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_B8,
    TYPE_B16,
    TYPE_B32,
    TYPE_B64,
    TYPE_RAW_PTR,
    TYPE_CUSTOM_LAYOUT
} DataTypeKind;

typedef struct DataType {
    DataTypeKind kind;
    int bit_width;
    bool is_signed;
    char name[64];
    int pointer_depth;
} DataType;

typedef enum {
    EXPR_INT_LIT,
    EXPR_STR_LIT,
    EXPR_VAR,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_INTRINSIC,
    EXPR_FIELD_ACCESS
} ExprKind;

typedef struct Expr Expr;

struct Expr {
    ExprKind kind;
    SourceLoc loc;
    DataType result_type;
    union {
        struct {
            uint64_t val;
            int bit_width;
            bool is_signed;
        } int_lit;
        struct {
            char text[256];
            int rodata_offset;
        } str_lit;
        struct {
            char name[64];
            int local_offset; // relative to RBP
        } var;
        struct {
            TokenType op;
            Expr *left;
            Expr *right;
        } binary;
        struct {
            TokenType op;
            Expr *operand;
        } unary;
        struct {
            char callee[64];
            Expr **args;
            int arg_count;
        } call;
        struct {
            char name[64];
            Expr **args;
            int arg_count;
        } intrinsic;
        struct {
            Expr *target;
            char field[64];
        } field;
    } as;
};

typedef enum {
    STMT_BIND,       // bind x: u32 = 10;
    STMT_MUT,        // mut y: u64 = 0;
    STMT_ASSIGN,     // y = y +% 1;
    STMT_BRANCH,     // branch (cond) { ... } otherwise { ... }
    STMT_CYCLE,      // cycle (init; cond; step) { ... }
    STMT_REPEAT,     // repeat { ... } until (cond);
    STMT_YIELD,      // yield expr;
    STMT_HALT,       // halt;
    STMT_TRAP,       // trap code;
    STMT_VERIFY,     // verify expr;
    STMT_EXPR        // expr;
} StmtKind;

typedef struct Stmt Stmt;

typedef struct Block {
    Stmt **stmts;
    int count;
    int capacity;
} Block;

struct Stmt {
    StmtKind kind;
    SourceLoc loc;
    union {
        struct {
            char name[64];
            DataType type;
            Expr *init;
            bool is_mutable;
            int stack_offset;
        } decl;
        struct {
            char name[64];
            Expr *val;
            int stack_offset;
        } assign;
        struct {
            Expr *cond;
            Block then_block;
            Block else_block;
            bool has_else;
        } branch;
        struct {
            Stmt *init;
            Expr *cond;
            Stmt *step;
            Block body;
        } cycle;
        struct {
            Block body;
            Expr *cond;
        } repeat_until;
        struct {
            Expr *val;
        } yield_stmt;
        struct {
            uint8_t code;
        } trap_stmt;
        struct {
            Expr *expr;
        } verify_stmt;
        struct {
            Expr *expr;
        } expr_stmt;
    } as;
};

typedef struct {
    char name[64];
    DataType type;
} Param;

typedef struct {
    char name[64];
    DataType type;
    int bit_offset;
    int bit_width;
} LayoutField;

typedef struct {
    char name[64];
    LayoutField fields[32];
    int field_count;
    int total_bytes;
    int alignment;
} LayoutDef;

typedef struct {
    char name[64];
    Param params[16];
    int param_count;
    DataType return_type;
    Block body;
    bool is_kernel_entry;
    int total_stack_bytes;
} Routine;

typedef struct {
    char system_name[64];
    char capabilities[16][64];
    int capability_count;
    LayoutDef layouts[32];
    int layout_count;
    Routine routines[64];
    int routine_count;
} AstProgram;

Expr *ast_new_expr(ExprKind kind, SourceLoc loc);
Stmt *ast_new_stmt(StmtKind kind, SourceLoc loc);
DataType parse_type_name(const char *name);
const char *data_type_to_string(DataType t);

#endif
