#include "parser.h"
#include <stdlib.h>
#include <string.h>

static void advance(Parser *p) {
    p->previous = p->current;
    p->current = lexer_next(&p->lexer);
}

static bool check(Parser *p, TokenType type) {
    return p->current.type == type;
}

static bool match(Parser *p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }
    return false;
}

static void consume(Parser *p, TokenType type, const char *err_msg) {
    if (check(p, type)) {
        advance(p);
        return;
    }
    diag_report(SEV_ERROR, "P0001", p->current.loc, "%s (got '%s')", err_msg, p->current.text);
    p->had_error = true;
}

void parser_init(Parser *p, const char *source, const char *filename) {
    lexer_init(&p->lexer, source, filename);
    p->had_error = false;
    advance(p); // prime first token
}

static Expr *parse_expression(Parser *p);
static Block parse_block(Parser *p);
static Stmt *parse_statement(Parser *p);

static Expr *parse_primary(Parser *p) {
    SourceLoc loc = p->current.loc;

    if (match(p, TOK_INT_LIT)) {
        Expr *e = ast_new_expr(EXPR_INT_LIT, loc);
        e->as.int_lit.val = p->previous.int_val;
        e->as.int_lit.bit_width = p->previous.bit_width;
        e->as.int_lit.is_signed = p->previous.is_signed;
        e->result_type.bit_width = p->previous.bit_width;
        e->result_type.is_signed = p->previous.is_signed;
        e->result_type.kind = p->previous.is_signed ? TYPE_I64 : TYPE_U64;
        return e;
    }

    if (match(p, TOK_STR_LIT)) {
        Expr *e = ast_new_expr(EXPR_STR_LIT, loc);
        strncpy(e->as.str_lit.text, p->previous.text, sizeof(e->as.str_lit.text) - 1);
        e->result_type.kind = TYPE_RAW_PTR;
        e->result_type.bit_width = 64;
        return e;
    }

    if (match(p, TOK_INTRINSIC)) {
        Expr *e = ast_new_expr(EXPR_INTRINSIC, loc);
        strncpy(e->as.intrinsic.name, p->previous.text, sizeof(e->as.intrinsic.name) - 1);
        consume(p, TOK_LPAREN, "Expected '(' after intrinsic");
        Expr *args[16];
        int count = 0;
        if (!check(p, TOK_RPAREN)) {
            do {
                if (count < 16) {
                    args[count++] = parse_expression(p);
                } else {
                    parse_expression(p);
                }
            } while (match(p, TOK_COMMA));
        }
        consume(p, TOK_RPAREN, "Expected ')' after intrinsic arguments");
        e->as.intrinsic.arg_count = count;
        e->as.intrinsic.args = (Expr **)malloc(sizeof(Expr *) * count);
        for (int i = 0; i < count; i++) e->as.intrinsic.args[i] = args[i];
        return e;
    }

    if (match(p, TOK_IDENT)) {
        char name[64];
        strncpy(name, p->previous.text, sizeof(name) - 1);

        // Check if function call
        if (match(p, TOK_LPAREN)) {
            Expr *e = ast_new_expr(EXPR_CALL, loc);
            strncpy(e->as.call.callee, name, sizeof(e->as.call.callee) - 1);
            Expr *args[16];
            int count = 0;
            if (!check(p, TOK_RPAREN)) {
                do {
                    if (count < 16) {
                        args[count++] = parse_expression(p);
                    } else {
                        parse_expression(p);
                    }
                } while (match(p, TOK_COMMA));
            }
            consume(p, TOK_RPAREN, "Expected ')' after routine arguments");
            e->as.call.arg_count = count;
            e->as.call.args = (Expr **)malloc(sizeof(Expr *) * count);
            for (int i = 0; i < count; i++) e->as.call.args[i] = args[i];
            return e;
        }

        Expr *e = ast_new_expr(EXPR_VAR, loc);
        strncpy(e->as.var.name, name, sizeof(e->as.var.name) - 1);
        return e;
    }

    if (match(p, TOK_LPAREN)) {
        Expr *e = parse_expression(p);
        consume(p, TOK_RPAREN, "Expected ')' after expression");
        return e;
    }

    diag_report(SEV_ERROR, "P0002", loc, "Unexpected token in expression: '%s'", p->current.text);
    p->had_error = true;
    advance(p);
    return ast_new_expr(EXPR_INT_LIT, loc);
}

static Expr *parse_unary(Parser *p) {
    if (match(p, TOK_OP_NOT) || match(p, TOK_OP_SUB)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *operand = parse_unary(p);
        Expr *e = ast_new_expr(EXPR_UNARY, loc);
        e->as.unary.op = op;
        e->as.unary.operand = operand;
        return e;
    }
    return parse_primary(p);
}

static Expr *parse_multiplicative(Parser *p) {
    Expr *expr = parse_unary(p);
    while (match(p, TOK_OP_MUL) || match(p, TOK_OP_MUL_CHECKED) || match(p, TOK_OP_MUL_WRAP) ||
           match(p, TOK_OP_DIV) || match(p, TOK_OP_MOD)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *right = parse_unary(p);
        Expr *bin = ast_new_expr(EXPR_BINARY, loc);
        bin->as.binary.op = op;
        bin->as.binary.left = expr;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

static Expr *parse_additive(Parser *p) {
    Expr *expr = parse_multiplicative(p);
    while (match(p, TOK_OP_ADD) || match(p, TOK_OP_ADD_CHECKED) || match(p, TOK_OP_ADD_WRAP) || match(p, TOK_OP_ADD_SAT) ||
           match(p, TOK_OP_SUB) || match(p, TOK_OP_SUB_CHECKED) || match(p, TOK_OP_SUB_WRAP) || match(p, TOK_OP_SUB_SAT)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *right = parse_multiplicative(p);
        Expr *bin = ast_new_expr(EXPR_BINARY, loc);
        bin->as.binary.op = op;
        bin->as.binary.left = expr;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

static Expr *parse_shift(Parser *p) {
    Expr *expr = parse_additive(p);
    while (match(p, TOK_OP_SHL) || match(p, TOK_OP_SHR) || match(p, TOK_OP_ROL) || match(p, TOK_OP_ROR)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *right = parse_additive(p);
        Expr *bin = ast_new_expr(EXPR_BINARY, loc);
        bin->as.binary.op = op;
        bin->as.binary.left = expr;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

static Expr *parse_bitwise(Parser *p) {
    Expr *expr = parse_shift(p);
    while (match(p, TOK_OP_AND) || match(p, TOK_OP_XOR) || match(p, TOK_OP_OR)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *right = parse_shift(p);
        Expr *bin = ast_new_expr(EXPR_BINARY, loc);
        bin->as.binary.op = op;
        bin->as.binary.left = expr;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

static Expr *parse_comparison(Parser *p) {
    Expr *expr = parse_bitwise(p);
    while (match(p, TOK_OP_EQ) || match(p, TOK_OP_NE) || match(p, TOK_OP_LT) ||
           match(p, TOK_OP_LE) || match(p, TOK_OP_GT) || match(p, TOK_OP_GE)) {
        TokenType op = p->previous.type;
        SourceLoc loc = p->previous.loc;
        Expr *right = parse_bitwise(p);
        Expr *bin = ast_new_expr(EXPR_BINARY, loc);
        bin->as.binary.op = op;
        bin->as.binary.left = expr;
        bin->as.binary.right = right;
        bin->result_type.bit_width = 8;
        expr = bin;
    }
    return expr;
}

static Expr *parse_expression(Parser *p) {
    return parse_comparison(p);
}

static Block parse_block(Parser *p) {
    consume(p, TOK_LBRACE, "Expected '{' to start block");
    Block blk;
    blk.count = 0;
    blk.capacity = 16;
    blk.stmts = (Stmt **)malloc(sizeof(Stmt *) * blk.capacity);

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        Stmt *s = parse_statement(p);
        if (s) {
            if (blk.count >= blk.capacity) {
                blk.capacity *= 2;
                blk.stmts = (Stmt **)realloc(blk.stmts, sizeof(Stmt *) * blk.capacity);
            }
            blk.stmts[blk.count++] = s;
        }
    }
    consume(p, TOK_RBRACE, "Expected '}' to close block");
    return blk;
}

static Stmt *parse_statement(Parser *p) {
    SourceLoc loc = p->current.loc;

    if (match(p, TOK_KW_BIND) || match(p, TOK_KW_MUT)) {
        bool is_mut = (p->previous.type == TOK_KW_MUT);
        consume(p, TOK_IDENT, "Expected identifier after bind/mut");
        char name[64];
        strncpy(name, p->previous.text, sizeof(name) - 1);

        consume(p, TOK_COLON, "Expected ':' after variable name");
        consume(p, TOK_IDENT, "Expected type name");
        DataType dt = parse_type_name(p->previous.text);

        consume(p, TOK_ASSIGN, "Expected '=' in variable initialization");
        Expr *init = parse_expression(p);
        consume(p, TOK_SEMICOLON, "Expected ';' after variable declaration");

        Stmt *s = ast_new_stmt(is_mut ? STMT_MUT : STMT_BIND, loc);
        strncpy(s->as.decl.name, name, sizeof(s->as.decl.name) - 1);
        s->as.decl.type = dt;
        s->as.decl.init = init;
        s->as.decl.is_mutable = is_mut;
        return s;
    }

    if (match(p, TOK_KW_BRANCH)) {
        consume(p, TOK_LPAREN, "Expected '(' after branch");
        Expr *cond = parse_expression(p);
        consume(p, TOK_RPAREN, "Expected ')' after branch condition");
        Block then_blk = parse_block(p);
        bool has_else = false;
        Block else_blk = {0};
        if (match(p, TOK_KW_OTHERWISE)) {
            has_else = true;
            else_blk = parse_block(p);
        }
        Stmt *s = ast_new_stmt(STMT_BRANCH, loc);
        s->as.branch.cond = cond;
        s->as.branch.then_block = then_blk;
        s->as.branch.else_block = else_blk;
        s->as.branch.has_else = has_else;
        return s;
    }

    if (match(p, TOK_KW_CYCLE)) {
        consume(p, TOK_LPAREN, "Expected '(' after cycle");
        Stmt *init = parse_statement(p);
        Expr *cond = parse_expression(p);
        consume(p, TOK_SEMICOLON, "Expected ';' after cycle condition");
        Stmt *step = parse_statement(p);
        consume(p, TOK_RPAREN, "Expected ')' after cycle header");
        Block body = parse_block(p);

        Stmt *s = ast_new_stmt(STMT_CYCLE, loc);
        s->as.cycle.init = init;
        s->as.cycle.cond = cond;
        s->as.cycle.step = step;
        s->as.cycle.body = body;
        return s;
    }

    if (match(p, TOK_KW_REPEAT)) {
        Block body = parse_block(p);
        consume(p, TOK_KW_UNTIL, "Expected 'until' after repeat block");
        consume(p, TOK_LPAREN, "Expected '(' after until");
        Expr *cond = parse_expression(p);
        consume(p, TOK_RPAREN, "Expected ')' after condition");
        consume(p, TOK_SEMICOLON, "Expected ';' after repeat until");

        Stmt *s = ast_new_stmt(STMT_REPEAT, loc);
        s->as.repeat_until.body = body;
        s->as.repeat_until.cond = cond;
        return s;
    }

    if (match(p, TOK_KW_YIELD)) {
        Expr *val = NULL;
        if (!check(p, TOK_SEMICOLON)) {
            val = parse_expression(p);
        }
        consume(p, TOK_SEMICOLON, "Expected ';' after yield");
        Stmt *s = ast_new_stmt(STMT_YIELD, loc);
        s->as.yield_stmt.val = val;
        return s;
    }

    if (match(p, TOK_KW_HALT)) {
        consume(p, TOK_SEMICOLON, "Expected ';' after halt");
        return ast_new_stmt(STMT_HALT, loc);
    }

    if (match(p, TOK_KW_TRAP)) {
        consume(p, TOK_INT_LIT, "Expected trap code literal");
        uint8_t code = (uint8_t)p->previous.int_val;
        consume(p, TOK_SEMICOLON, "Expected ';' after trap");
        Stmt *s = ast_new_stmt(STMT_TRAP, loc);
        s->as.trap_stmt.code = code;
        return s;
    }

    if (match(p, TOK_KW_VERIFY)) {
        Expr *e = parse_expression(p);
        consume(p, TOK_SEMICOLON, "Expected ';' after verify");
        Stmt *s = ast_new_stmt(STMT_VERIFY, loc);
        s->as.verify_stmt.expr = e;
        return s;
    }

    // Assignment or expression statement
    if (check(p, TOK_IDENT)) {
        advance(p);
        char id_name[64];
        strncpy(id_name, p->previous.text, sizeof(id_name) - 1);

        if (match(p, TOK_ASSIGN)) {
            Expr *val = parse_expression(p);
            // Semicolon is optional inside cycle headers
            match(p, TOK_SEMICOLON);
            Stmt *s = ast_new_stmt(STMT_ASSIGN, loc);
            strncpy(s->as.assign.name, id_name, sizeof(s->as.assign.name) - 1);
            s->as.assign.val = val;
            return s;
        }

        // Otherwise reconstruct as variable / call expression
        p->current = p->previous; // put back ident
        p->lexer.cursor -= (int)strlen(id_name);
        p->lexer.col -= (int)strlen(id_name);
        advance(p);
    }

    Expr *e = parse_expression(p);
    consume(p, TOK_SEMICOLON, "Expected ';' after expression statement");
    Stmt *s = ast_new_stmt(STMT_EXPR, loc);
    s->as.expr_stmt.expr = e;
    return s;
}

bool parser_parse(Parser *p, AstProgram *prog) {
    memset(prog, 0, sizeof(AstProgram));

    // Optional: system <name>;
    if (match(p, TOK_KW_SYSTEM)) {
        consume(p, TOK_IDENT, "Expected system name");
        strncpy(prog->system_name, p->previous.text, sizeof(prog->system_name) - 1);
        consume(p, TOK_SEMICOLON, "Expected ';' after system declaration");
    } else {
        strcpy(prog->system_name, "main_system");
    }

    while (!check(p, TOK_EOF)) {
        // Capabilities: require cap: <name>;
        if (match(p, TOK_KW_REQUIRE)) {
            consume(p, TOK_KW_CAP, "Expected 'cap' after require");
            consume(p, TOK_COLON, "Expected ':' after cap");
            consume(p, TOK_IDENT, "Expected capability name");
            if (prog->capability_count < 16) {
                strncpy(prog->capabilities[prog->capability_count++], p->previous.text, 63);
            }
            consume(p, TOK_SEMICOLON, "Expected ';' after require cap");
            continue;
        }

        // Layout: layout <name> { ... }
        if (match(p, TOK_KW_LAYOUT)) {
            consume(p, TOK_IDENT, "Expected layout name");
            LayoutDef *layout = &prog->layouts[prog->layout_count++];
            strncpy(layout->name, p->previous.text, sizeof(layout->name) - 1);
            consume(p, TOK_LBRACE, "Expected '{' to start layout");
            int offset = 0;
            while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
                consume(p, TOK_IDENT, "Expected field name");
                LayoutField *f = &layout->fields[layout->field_count++];
                strncpy(f->name, p->previous.text, sizeof(f->name) - 1);
                consume(p, TOK_COLON, "Expected ':' after field name");
                consume(p, TOK_IDENT, "Expected field type");
                f->type = parse_type_name(p->previous.text);
                f->bit_width = f->type.bit_width;
                if (match(p, TOK_AT)) {
                    consume(p, TOK_INT_LIT, "Expected explicit offset");
                    f->bit_offset = (int)p->previous.int_val * 8;
                } else {
                    f->bit_offset = offset * 8;
                }
                offset += (f->bit_width / 8);
                consume(p, TOK_COMMA, "Expected ',' after layout field");
            }
            consume(p, TOK_RBRACE, "Expected '}' to close layout");
            layout->total_bytes = offset;
            layout->alignment = 8;
            continue;
        }

        // Routines: routine <name>(...) -> <ret> { ... }
        // or kernel_entry <name>() -> void { ... }
        bool is_kernel = match(p, TOK_KW_KERNEL_ENTRY);
        if (is_kernel || match(p, TOK_KW_ROUTINE)) {
            consume(p, TOK_IDENT, "Expected routine name");
            Routine *r = &prog->routines[prog->routine_count++];
            strncpy(r->name, p->previous.text, sizeof(r->name) - 1);
            r->is_kernel_entry = is_kernel;

            consume(p, TOK_LPAREN, "Expected '(' after routine name");
            if (!check(p, TOK_RPAREN)) {
                do {
                    consume(p, TOK_IDENT, "Expected parameter name");
                    Param *param = &r->params[r->param_count++];
                    strncpy(param->name, p->previous.text, sizeof(param->name) - 1);
                    consume(p, TOK_COLON, "Expected ':' after parameter name");
                    consume(p, TOK_IDENT, "Expected parameter type");
                    param->type = parse_type_name(p->previous.text);
                } while (match(p, TOK_COMMA));
            }
            consume(p, TOK_RPAREN, "Expected ')' after parameters");

            if (match(p, TOK_ARROW)) {
                consume(p, TOK_IDENT, "Expected return type");
                r->return_type = parse_type_name(p->previous.text);
            } else {
                r->return_type.kind = TYPE_VOID;
            }

            r->body = parse_block(p);
            continue;
        }

        diag_report(SEV_ERROR, "P0003", p->current.loc, "Unexpected top-level token: '%s'", p->current.text);
        p->had_error = true;
        advance(p);
    }

    return !p->had_error;
}
