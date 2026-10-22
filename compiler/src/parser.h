#ifndef I_PARSER_H
#define I_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    bool had_error;
} Parser;

void parser_init(Parser *p, const char *source, const char *filename);
bool parser_parse(Parser *p, AstProgram *out_prog);

#endif
