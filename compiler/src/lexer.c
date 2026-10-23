#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

void lexer_init(Lexer *lex, const char *source, const char *filename) {
    lex->source = source;
    lex->cursor = 0;
    lex->line = 1;
    lex->col = 1;
    lex->filename = filename;
}

static char peek(Lexer *lex) {
    return lex->source[lex->cursor];
}

static char peek_next(Lexer *lex) {
    if (!lex->source[lex->cursor]) return '\0';
    return lex->source[lex->cursor + 1];
}

static char advance(Lexer *lex) {
    char c = lex->source[lex->cursor++];
    if (c == '\n') {
        lex->line++;
        lex->col = 1;
    } else {
        lex->col++;
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer *lex) {
    while (peek(lex)) {
        char c = peek(lex);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lex);
        } else if (c == '/' && peek_next(lex) == '/') {
            while (peek(lex) && peek(lex) != '\n') advance(lex);
        } else if (c == '/' && peek_next(lex) == '*') {
            advance(lex); advance(lex);
            while (peek(lex) && !(peek(lex) == '*' && peek_next(lex) == '/')) {
                advance(lex);
            }
            if (peek(lex)) { advance(lex); advance(lex); }
        } else if (c == '#' && peek_next(lex) != '[') {
            // Shell / hash comment line
            while (peek(lex) && peek(lex) != '\n') advance(lex);
        } else {
            break;
        }
    }
}

static Token make_token(TokenType type, const char *text, SourceLoc loc) {
    Token tok;
    tok.type = type;
    strncpy(tok.text, text, sizeof(tok.text) - 1);
    tok.text[sizeof(tok.text) - 1] = '\0';
    tok.int_val = 0;
    tok.bit_width = 64;
    tok.is_signed = false;
    tok.loc = loc;
    return tok;
}

static TokenType match_keyword(const char *str) {
    if (strcmp(str, "system") == 0) return TOK_KW_SYSTEM;
    if (strcmp(str, "system") == 0) return TOK_KW_SYSTEM;
    if (strcmp(str, "require") == 0) return TOK_KW_REQUIRE;
    if (strcmp(str, "cap") == 0) return TOK_KW_CAP;
    if (strcmp(str, "use") == 0) return TOK_KW_USE;

    // Routine / function aliases (fn, func, routine)
    if (strcmp(str, "routine") == 0 || strcmp(str, "fn") == 0 || strcmp(str, "func") == 0) return TOK_KW_ROUTINE;
    if (strcmp(str, "kernel_entry") == 0) return TOK_KW_KERNEL_ENTRY;

    // Variable binding aliases (let, val, bind, mut, var)
    if (strcmp(str, "bind") == 0 || strcmp(str, "let") == 0 || strcmp(str, "val") == 0) return TOK_KW_BIND;
    if (strcmp(str, "mut") == 0 || strcmp(str, "var") == 0) return TOK_KW_MUT;

    // Conditionals (if/else, branch/otherwise)
    if (strcmp(str, "branch") == 0 || strcmp(str, "if") == 0) return TOK_KW_BRANCH;
    if (strcmp(str, "otherwise") == 0 || strcmp(str, "else") == 0) return TOK_KW_OTHERWISE;

    // Loops (while, for, cycle)
    if (strcmp(str, "cycle") == 0 || strcmp(str, "while") == 0 || strcmp(str, "for") == 0) return TOK_KW_CYCLE;
    if (strcmp(str, "repeat") == 0) return TOK_KW_REPEAT;
    if (strcmp(str, "until") == 0) return TOK_KW_UNTIL;

    // Control flow (return, yield)
    if (strcmp(str, "yield") == 0 || strcmp(str, "return") == 0) return TOK_KW_YIELD;
    if (strcmp(str, "halt") == 0) return TOK_KW_HALT;
    if (strcmp(str, "trap") == 0) return TOK_KW_TRAP;

    // Assertions (verify, assert)
    if (strcmp(str, "verify") == 0 || strcmp(str, "assert") == 0) return TOK_KW_VERIFY;
    if (strcmp(str, "atomic") == 0) return TOK_KW_ATOMIC;
    if (strcmp(str, "unsafe") == 0) return TOK_KW_UNSAFE;
    if (strcmp(str, "layout") == 0 || strcmp(str, "struct") == 0) return TOK_KW_LAYOUT;
    if (strcmp(str, "bitfield") == 0) return TOK_KW_BITFIELD;
    return TOK_IDENT;
}

Token lexer_next(Lexer *lex) {
    skip_whitespace_and_comments(lex);

    SourceLoc loc = { .line = lex->line, .col = lex->col, .file = lex->filename };

    if (!peek(lex)) {
        return make_token(TOK_EOF, "", loc);
    }

    char c = peek(lex);

    // Intrinsic identifier: @identifier
    if (c == '@' && (isalpha(peek_next(lex)) || peek_next(lex) == '_')) {
        char buf[128] = {0};
        int idx = 0;
        buf[idx++] = advance(lex);
        while (isalnum(peek(lex)) || peek(lex) == '_') {
            if (idx < 127) buf[idx++] = advance(lex);
            else advance(lex);
        }
        return make_token(TOK_INTRINSIC, buf, loc);
    }

    // Identifiers and Keywords
    if (isalpha(c) || c == '_') {
        char buf[128] = {0};
        int idx = 0;
        while (isalnum(peek(lex)) || peek(lex) == '_') {
            if (idx < 127) buf[idx++] = advance(lex);
            else advance(lex);
        }
        TokenType kw = match_keyword(buf);
        return make_token(kw, buf, loc);
    }

    // Number literals (Binary 0b..., Hex 0x..., Decimal)
    if (isdigit(c)) {
        char buf[128] = {0};
        int idx = 0;
        uint64_t val = 0;
        int base = 10;

        if (c == '0' && (peek_next(lex) == 'b' || peek_next(lex) == 'B')) {
            base = 2;
            buf[idx++] = advance(lex); // 0
            buf[idx++] = advance(lex); // b
            while (peek(lex) == '0' || peek(lex) == '1' || peek(lex) == '_') {
                char ch = advance(lex);
                if (ch != '_') {
                    val = (val << 1) | (ch - '0');
                    if (idx < 127) buf[idx++] = ch;
                }
            }
        } else if (c == '0' && (peek_next(lex) == 'x' || peek_next(lex) == 'X')) {
            base = 16;
            buf[idx++] = advance(lex); // 0
            buf[idx++] = advance(lex); // x
            while (isxdigit(peek(lex)) || peek(lex) == '_') {
                char ch = advance(lex);
                if (ch != '_') {
                    int digit = isdigit(ch) ? (ch - '0') : (tolower(ch) - 'a' + 10);
                    val = (val << 4) | digit;
                    if (idx < 127) buf[idx++] = ch;
                }
            }
        } else {
            while (isdigit(peek(lex)) || peek(lex) == '_') {
                char ch = advance(lex);
                if (ch != '_') {
                    val = (val * 10) + (ch - '0');
                    if (idx < 127) buf[idx++] = ch;
                }
            }
        }

        int width = 64;
        bool is_signed = false;
        // Check for bit/type suffix: e.g. :u8, :u16, :u32, :u64, :i8, :i16, :i32, :i64, :b8
        if (peek(lex) == ':') {
            advance(lex);
            char sbuf[32] = {0};
            int sidx = 0;
            while (isalnum(peek(lex))) {
                if (sidx < 31) sbuf[sidx++] = advance(lex);
            }
            if (strcmp(sbuf, "u8") == 0 || strcmp(sbuf, "b8") == 0) width = 8;
            else if (strcmp(sbuf, "u16") == 0 || strcmp(sbuf, "b16") == 0) width = 16;
            else if (strcmp(sbuf, "u32") == 0 || strcmp(sbuf, "b32") == 0) width = 32;
            else if (strcmp(sbuf, "u64") == 0 || strcmp(sbuf, "b64") == 0) width = 64;
            else if (strcmp(sbuf, "i8") == 0) { width = 8; is_signed = true; }
            else if (strcmp(sbuf, "i16") == 0) { width = 16; is_signed = true; }
            else if (strcmp(sbuf, "i32") == 0) { width = 32; is_signed = true; }
            else if (strcmp(sbuf, "i64") == 0) { width = 64; is_signed = true; }
        }

        Token tok = make_token(TOK_INT_LIT, buf, loc);
        tok.int_val = val;
        tok.bit_width = width;
        tok.is_signed = is_signed;
        return tok;
    }

    // String literals
    if (c == '"') {
        advance(lex); // skip opening "
        char buf[512] = {0};
        int idx = 0;
        while (peek(lex) && peek(lex) != '"') {
            char ch = advance(lex);
            if (ch == '\\') {
                char esc = advance(lex);
                if (esc == 'n') ch = '\n';
                else if (esc == 't') ch = '\t';
                else if (esc == 'r') ch = '\r';
                else if (esc == '0') ch = '\0';
            }
            if (idx < 511) buf[idx++] = ch;
        }
        if (peek(lex) == '"') advance(lex);
        return make_token(TOK_STR_LIT, buf, loc);
    }

    // Multi-character operators
    advance(lex); // consume first char

    if (c == '+' && peek(lex) == '!') { advance(lex); return make_token(TOK_OP_ADD_CHECKED, "+!", loc); }
    if (c == '-' && peek(lex) == '!') { advance(lex); return make_token(TOK_OP_SUB_CHECKED, "-!", loc); }
    if (c == '*' && peek(lex) == '!') { advance(lex); return make_token(TOK_OP_MUL_CHECKED, "*!", loc); }

    if (c == '+' && peek(lex) == '%') { advance(lex); return make_token(TOK_OP_ADD_WRAP, "+%", loc); }
    if (c == '-' && peek(lex) == '%') { advance(lex); return make_token(TOK_OP_SUB_WRAP, "-%", loc); }
    if (c == '*' && peek(lex) == '%') { advance(lex); return make_token(TOK_OP_MUL_WRAP, "*%", loc); }

    if (c == '+' && peek(lex) == '?') { advance(lex); return make_token(TOK_OP_ADD_SAT, "+?", loc); }
    if (c == '-' && peek(lex) == '?') { advance(lex); return make_token(TOK_OP_SUB_SAT, "-?", loc); }

    if (c == '<' && peek(lex) == '<' && peek_next(lex) == '<') { advance(lex); advance(lex); return make_token(TOK_OP_ROL, "<<<", loc); }
    if (c == '>' && peek(lex) == '>' && peek_next(lex) == '>') { advance(lex); advance(lex); return make_token(TOK_OP_ROR, ">>>", loc); }

    if (c == '<' && peek(lex) == '<') { advance(lex); return make_token(TOK_OP_SHL, "<<", loc); }
    if (c == '>' && peek(lex) == '>') { advance(lex); return make_token(TOK_OP_SHR, ">>", loc); }

    if (c == '=' && peek(lex) == '=') { advance(lex); return make_token(TOK_OP_EQ, "==", loc); }
    if (c == '!' && peek(lex) == '=') { advance(lex); return make_token(TOK_OP_NE, "!=", loc); }
    if (c == '<' && peek(lex) == '=') { advance(lex); return make_token(TOK_OP_LE, "<=", loc); }
    if (c == '>' && peek(lex) == '=') { advance(lex); return make_token(TOK_OP_GE, ">=", loc); }

    if (c == '-' && peek(lex) == '>') { advance(lex); return make_token(TOK_ARROW, "->", loc); }
    if (c == ':' && peek(lex) == ':') { advance(lex); return make_token(TOK_DBL_COLON, "::", loc); }

    // Single-char tokens
    switch (c) {
        case '+': return make_token(TOK_OP_ADD, "+", loc);
        case '-': return make_token(TOK_OP_SUB, "-", loc);
        case '*': return make_token(TOK_OP_MUL, "*", loc);
        case '/': return make_token(TOK_OP_DIV, "/", loc);
        case '%': return make_token(TOK_OP_MOD, "%", loc);
        case '&': return make_token(TOK_OP_AND, "&", loc);
        case '|': return make_token(TOK_OP_OR, "|", loc);
        case '^': return make_token(TOK_OP_XOR, "^", loc);
        case '~': return make_token(TOK_OP_NOT, "~", loc);
        case '<': return make_token(TOK_OP_LT, "<", loc);
        case '>': return make_token(TOK_OP_GT, ">", loc);
        case '=': return make_token(TOK_ASSIGN, "=", loc);
        case ':': return make_token(TOK_COLON, ":", loc);
        case ';': return make_token(TOK_SEMICOLON, ";", loc);
        case ',': return make_token(TOK_COMMA, ",", loc);
        case '.': return make_token(TOK_DOT, ".", loc);
        case '@': return make_token(TOK_AT, "@", loc);
        case '(': return make_token(TOK_LPAREN, "(", loc);
        case ')': return make_token(TOK_RPAREN, ")", loc);
        case '{': return make_token(TOK_LBRACE, "{", loc);
        case '}': return make_token(TOK_RBRACE, "}", loc);
        case '[': return make_token(TOK_LBRACKET, "[", loc);
        case ']': return make_token(TOK_RBRACKET, "]", loc);
        default: {
            char bad[2] = { c, '\0' };
            diag_report(SEV_ERROR, "E0001", loc, "Unexpected character '%c' (0x%02X)", c, (unsigned char)c);
            return make_token(TOK_EOF, bad, loc);
        }
    }
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_EOF: return "EOF";
        case TOK_IDENT: return "IDENT";
        case TOK_INT_LIT: return "INT_LIT";
        case TOK_STR_LIT: return "STR_LIT";
        case TOK_KW_SYSTEM: return "system";
        case TOK_KW_ROUTINE: return "routine";
        case TOK_KW_KERNEL_ENTRY: return "kernel_entry";
        case TOK_KW_YIELD: return "yield";
        case TOK_KW_BIND: return "bind";
        case TOK_KW_MUT: return "mut";
        case TOK_KW_BRANCH: return "branch";
        case TOK_KW_CYCLE: return "cycle";
        case TOK_INTRINSIC: return "INTRINSIC";
        default: return "TOKEN";
    }
}
