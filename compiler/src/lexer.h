#ifndef I_LEXER_H
#define I_LEXER_H

#include "diagnostics.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_INT_LIT,
    TOK_STR_LIT,

    // Keywords
    TOK_KW_SYSTEM,
    TOK_KW_REQUIRE,
    TOK_KW_CAP,
    TOK_KW_USE,
    TOK_KW_ROUTINE,
    TOK_KW_KERNEL_ENTRY,
    TOK_KW_BIND,
    TOK_KW_MUT,
    TOK_KW_BRANCH,
    TOK_KW_OTHERWISE,
    TOK_KW_CYCLE,
    TOK_KW_REPEAT,
    TOK_KW_UNTIL,
    TOK_KW_YIELD,
    TOK_KW_HALT,
    TOK_KW_TRAP,
    TOK_KW_VERIFY,
    TOK_KW_ATOMIC,
    TOK_KW_UNSAFE,
    TOK_KW_LAYOUT,
    TOK_KW_BITFIELD,

    // Operators
    TOK_OP_ADD_CHECKED, // +!
    TOK_OP_SUB_CHECKED, // -!
    TOK_OP_MUL_CHECKED, // *!
    TOK_OP_ADD_WRAP,    // +%
    TOK_OP_SUB_WRAP,    // -%
    TOK_OP_MUL_WRAP,    // *%
    TOK_OP_ADD_SAT,     // +?
    TOK_OP_SUB_SAT,     // -?
    TOK_OP_ADD,         // +
    TOK_OP_SUB,         // -
    TOK_OP_MUL,         // *
    TOK_OP_DIV,         // /
    TOK_OP_MOD,         // %
    TOK_OP_AND,         // &
    TOK_OP_OR,          // |
    TOK_OP_XOR,         // ^
    TOK_OP_NOT,         // ~
    TOK_OP_SHL,         // <<
    TOK_OP_SHR,         // >>
    TOK_OP_ROL,         // <<<
    TOK_OP_ROR,         // >>>

    // Comparisons
    TOK_OP_EQ,          // ==
    TOK_OP_NE,          // !=
    TOK_OP_LT,          // <
    TOK_OP_LE,          // <=
    TOK_OP_GT,          // >
    TOK_OP_GE,          // >=

    // Punctuation
    TOK_ASSIGN,         // =
    TOK_ARROW,          // ->
    TOK_COLON,          // :
    TOK_DBL_COLON,      // ::
    TOK_SEMICOLON,      // ;
    TOK_COMMA,          // ,
    TOK_DOT,            // .
    TOK_AT,             // @
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_LBRACE,         // {
    TOK_RBRACE,         // }
    TOK_LBRACKET,       // [
    TOK_RBRACKET,       // ]
    TOK_INTRINSIC       // @name
} TokenType;

typedef struct {
    TokenType type;
    char text[128];
    uint64_t int_val;
    int bit_width;      // e.g. 8, 16, 32, 64
    bool is_signed;
    SourceLoc loc;
} Token;

typedef struct {
    const char *source;
    int cursor;
    int line;
    int col;
    const char *filename;
} Lexer;

void lexer_init(Lexer *lex, const char *source, const char *filename);
Token lexer_next(Lexer *lex);
const char *token_type_name(TokenType type);

#endif
