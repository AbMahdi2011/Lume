#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String duplication helper for C99
static char *custom_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);
    if (dup) memcpy(dup, s, len);
    return dup;
}

/* =========================================================================
 * Helper Functions & Error Handling
 * ========================================================================= */

static void parser_error(Parser *p, const char *message) {
    p->had_error = 1;
    fprintf(stderr, "[Line %d, Col %d] Syntax Error: %s (got '%s')\n",
            p->current.line, p->current.col, message, p->current.lexeme);
}

static void advance(Parser *p) {
    token_free(&p->previous);
    p->previous = p->current;
    p->current = lexer_next_token(&p->lexer);
}

static int check(const Parser *p, TokenType type) {
    return p->current.type == type;
}

static int match(Parser *p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return 1;
    }
    return 0;
}

static void consume(Parser *p, TokenType type, const char *message) {
    if (check(p, type)) {
        advance(p);
        return;
    }
    parser_error(p, message);
}

/* =========================================================================
 * Forward Declarations
 * ========================================================================= */
static Type *parse_type(Parser *p);
static Expr *parse_expression(Parser *p);
static Expr *parse_assignment(Parser *p);
static Stmt *parse_statement(Parser *p);
static Stmt *parse_block(Parser *p);
static Decl *parse_declaration(Parser *p);

/* =========================================================================
 * Type Parsing
 * ========================================================================= */
static int is_type_start(const Parser *p) {
    return check(p, TOK_KW_INT) ||
           check(p, TOK_KW_CHAR) ||
           check(p, TOK_KW_VOID) ||
           check(p, TOK_KW_STRUCT);
}

static Type *parse_type(Parser *p) {
    Type *base_type = NULL;

    if (match(p, TOK_KW_INT)) {
        base_type = type_primitive(TYPE_INT);
    } else if (match(p, TOK_KW_CHAR)) {
        base_type = type_primitive(TYPE_CHAR);
    } else if (match(p, TOK_KW_VOID)) {
        base_type = type_primitive(TYPE_VOID);
    } else if (match(p, TOK_KW_STRUCT)) {
        consume(p, TOK_IDENT, "Expected struct name after 'struct'");
        base_type = type_struct(p->previous.lexeme);
    } else {
        parser_error(p, "Expected type name");
        return type_primitive(TYPE_VOID);
    }

    return base_type;
}

/* =========================================================================
 * Expression Parsing (Recursive Descent & Precedence)
 * ========================================================================= */

// Primary: Literals, Variables, Parentheses
static Expr *parse_primary(Parser *p) {
    int line = p->current.line;
    int col = p->current.col;

    if (match(p, TOK_INT_LIT)) {
        return expr_int_lit(atoi(p->previous.lexeme), line, col);
    }

    if (match(p, TOK_STR_LIT)) {
        return expr_str_lit(p->previous.lexeme, line, col);
    }

    if (match(p, TOK_IDENT)) {
        return expr_var(p->previous.lexeme, line, col);
    }

    if (match(p, TOK_LPAREN)) {
        Expr *expr = parse_expression(p);
        consume(p, TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }

    parser_error(p, "Expected expression");
    return expr_int_lit(0, line, col);
}

// Postfix: Function Calls f(...), Array Indexing a[i], Member Access s.f
static Expr *parse_postfix(Parser *p) {
    Expr *expr = parse_primary(p);

    while (1) {
        int line = p->current.line;
        int col = p->current.col;

        // Function Call
        if (match(p, TOK_LPAREN)) {
            Expr **args = NULL;
            int arg_count = 0;

            if (!check(p, TOK_RPAREN)) {
                do {
                    args = (Expr **)realloc(args, sizeof(Expr *) * (arg_count + 1));
                    args[arg_count++] = parse_expression(p);
                } while (match(p, TOK_COMMA));
            }
            consume(p, TOK_RPAREN, "Expected ')' after argument list");

            if (expr->kind != EXPR_VAR) {
                parser_error(p, "Expression is not callable");
            }
            char *callee = expr->as.var_name;
            expr->as.var_name = NULL;
            expr_free(expr);

            expr = expr_call(callee, args, arg_count, line, col);
            free(callee);
        }
        // Array Indexing
        else if (match(p, TOK_LBRACKET)) {
            Expr *index = parse_expression(p);
            consume(p, TOK_RBRACKET, "Expected ']' after array index");
            expr = expr_index(expr, index, line, col);
        }
        // Member Access
        else if (match(p, TOK_DOT)) {
            consume(p, TOK_IDENT, "Expected member name after '.'");
            expr = expr_member(expr, p->previous.lexeme, line, col);
        } else {
            break;
        }
    }

    return expr;
}

// Unary: -x, !x
static Expr *parse_unary(Parser *p) {
    if (check(p, TOK_MINUS) || check(p, TOK_BANG)) {
        advance(p);
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *operand = parse_unary(p);
        return expr_unary(op, operand, line, col);
    }
    return parse_postfix(p);
}

// Multiplicative: *, /, %
static Expr *parse_multiplicative(Parser *p) {
    Expr *left = parse_unary(p);

    while (check(p, TOK_STAR) || check(p, TOK_SLASH) || check(p, TOK_PERCENT)) {
        advance(p);
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_unary(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

// Additive: +, -
static Expr *parse_additive(Parser *p) {
    Expr *left = parse_multiplicative(p);

    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        advance(p);
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_multiplicative(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

// Relational: <, <=, >, >=
static Expr *parse_relational(Parser *p) {
    Expr *left = parse_additive(p);

    while (check(p, TOK_LT) || check(p, TOK_LT_EQ) ||
           check(p, TOK_GT) || check(p, TOK_GT_EQ)) {
        advance(p);
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_additive(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

// Equality: ==, !=
static Expr *parse_equality(Parser *p) {
    Expr *left = parse_relational(p);

    while (check(p, TOK_EQ_EQ) || check(p, TOK_BANG_EQ)) {
        advance(p);
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_relational(p);
        left = expr_binary(op, left, right, line, col);
    }
    return left;
}

// Logical AND: &&
static Expr *parse_logical_and(Parser *p) {
    Expr *left = parse_equality(p);

    while (match(p, TOK_AND_AND)) {
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_equality(p);
        left = expr_binary(TOK_AND_AND, left, right, line, col);
    }
    return left;
}

// Logical OR: ||
static Expr *parse_logical_or(Parser *p) {
    Expr *left = parse_logical_and(p);

    while (match(p, TOK_OR_OR)) {
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *right = parse_logical_and(p);
        left = expr_binary(TOK_OR_OR, left, right, line, col);
    }
    return left;
}

// Assignment: lhs = rhs
static Expr *parse_assignment(Parser *p) {
    Expr *expr = parse_logical_or(p);

    if (match(p, TOK_ASSIGN)) {
        int line = p->previous.line;
        int col = p->previous.col;
        Expr *value = parse_assignment(p); // Right-associative

        if (expr->kind != EXPR_VAR &&
            expr->kind != EXPR_INDEX &&
            expr->kind != EXPR_MEMBER) {
            parser_error(p, "Invalid assignment target (l-value required)");
        }
        return expr_assign(expr, value, line, col);
    }

    return expr;
}

static Expr *parse_expression(Parser *p) {
    return parse_assignment(p);
}

/* =========================================================================
 * Statement Parsing
 * ========================================================================= */

// Block: { stmt1; stmt2; ... }
static Stmt *parse_block(Parser *p) {
    consume(p, TOK_LBRACE, "Expected '{' to start block");

    Stmt **stmts = NULL;
    int count = 0;

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        stmts = (Stmt **)realloc(stmts, sizeof(Stmt *) * (count + 1));
        stmts[count++] = parse_statement(p);
    }

    consume(p, TOK_RBRACE, "Expected '}' after block");
    return stmt_block(stmts, count);
}

// Variable Declaration: int x = 5; or int arr[10]; or struct Point p;
static Stmt *parse_var_decl(Parser *p) {
    Type *base_type = parse_type(p);
    consume(p, TOK_IDENT, "Expected variable name");
    char *name = custom_strdup(p->previous.lexeme);

    Type *final_type = base_type;
    // Check if it's an array: int arr[10];
    if (match(p, TOK_LBRACKET)) {
        consume(p, TOK_INT_LIT, "Expected integer array size");
        int size = atoi(p->previous.lexeme);
        consume(p, TOK_RBRACKET, "Expected ']' after array size");
        final_type = type_array(base_type, size);
    }

    Expr *init_expr = NULL;
    if (match(p, TOK_ASSIGN)) {
        init_expr = parse_expression(p);
    }

    consume(p, TOK_SEMICOLON, "Expected ';' after variable declaration");
    Stmt *stmt = stmt_var_decl(final_type, name, init_expr);
    free(name);
    return stmt;
}

// If Statement: if (cond) stmt [else stmt]
static Stmt *parse_if(Parser *p) {
    consume(p, TOK_LPAREN, "Expected '(' after 'if'");
    Expr *cond = parse_expression(p);
    consume(p, TOK_RPAREN, "Expected ')' after if condition");

    Stmt *then_branch = parse_statement(p);
    Stmt *else_branch = NULL;

    if (match(p, TOK_KW_ELSE)) {
        else_branch = parse_statement(p);
    }

    return stmt_if(cond, then_branch, else_branch);
}

// While Statement: while (cond) stmt
static Stmt *parse_while(Parser *p) {
    consume(p, TOK_LPAREN, "Expected '(' after 'while'");
    Expr *cond = parse_expression(p);
    consume(p, TOK_RPAREN, "Expected ')' after while condition");

    Stmt *body = parse_statement(p);
    return stmt_while(cond, body);
}

// Return Statement: return [expr];
static Stmt *parse_return(Parser *p) {
    Expr *expr = NULL;
    if (!check(p, TOK_SEMICOLON)) {
        expr = parse_expression(p);
    }
    consume(p, TOK_SEMICOLON, "Expected ';' after return statement");
    return stmt_return(expr);
}

static Stmt *parse_statement(Parser *p) {
    if (check(p, TOK_LBRACE)) {
        return parse_block(p);
    }
    if (match(p, TOK_KW_IF)) {
        return parse_if(p);
    }
    if (match(p, TOK_KW_WHILE)) {
        return parse_while(p);
    }
    if (match(p, TOK_KW_RETURN)) {
        return parse_return(p);
    }
    if (is_type_start(p)) {
        return parse_var_decl(p);
    }

    // Otherwise, Expression Statement
    Expr *expr = parse_expression(p);
    consume(p, TOK_SEMICOLON, "Expected ';' after expression");
    return stmt_expr(expr);
}

/* =========================================================================
 * Top-Level Declaration Parsing (Struct, Fn, Directive)
 * ========================================================================= */

// Struct Declaration: struct Point { int x; int y; };
static Decl *parse_struct_decl(Parser *p) {
    consume(p, TOK_IDENT, "Expected struct name");
    char *name = custom_strdup(p->previous.lexeme);

    consume(p, TOK_LBRACE, "Expected '{' after struct name");

    StructField *fields = NULL;
    int field_count = 0;

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        Type *f_type = parse_type(p);
        consume(p, TOK_IDENT, "Expected field name");
        char *f_name = custom_strdup(p->previous.lexeme);

        // Check if field is array: int buf[32];
        if (match(p, TOK_LBRACKET)) {
            consume(p, TOK_INT_LIT, "Expected integer array size");
            int size = atoi(p->previous.lexeme);
            consume(p, TOK_RBRACKET, "Expected ']' after array size");
            f_type = type_array(f_type, size);
        }

        consume(p, TOK_SEMICOLON, "Expected ';' after struct field");

        fields = (StructField *)realloc(fields, sizeof(StructField) * (field_count + 1));
        fields[field_count].type = f_type;
        fields[field_count].name = f_name;
        field_count++;
    }

    consume(p, TOK_RBRACE, "Expected '}' after struct body");
    match(p, TOK_SEMICOLON); // Semicolon after struct is optional in Lume

    Decl *decl = decl_struct(name, fields, field_count);
    free(name);
    return decl;
}

// Function Declaration: fn name(int a, int b): int { ... }
static Decl *parse_function_decl(Parser *p) {
    consume(p, TOK_IDENT, "Expected function name");
    char *name = custom_strdup(p->previous.lexeme);

    consume(p, TOK_LPAREN, "Expected '(' after function name");

    Param *params = NULL;
    int param_count = 0;

    if (!check(p, TOK_RPAREN)) {
        do {
            Type *p_type = parse_type(p);
            consume(p, TOK_IDENT, "Expected parameter name");
            char *p_name = custom_strdup(p->previous.lexeme);

            params = (Param *)realloc(params, sizeof(Param) * (param_count + 1));
            params[param_count].type = p_type;
            params[param_count].name = p_name;
            param_count++;
        } while (match(p, TOK_COMMA));
    }

    consume(p, TOK_RPAREN, "Expected ')' after parameter list");

    Type *ret_type = NULL;
    if (match(p, TOK_COLON)) {
        ret_type = parse_type(p);
    } else {
        ret_type = type_primitive(TYPE_VOID);
    }

    Stmt *body = parse_block(p);

    Decl *decl = decl_function(name, params, param_count, ret_type, body);
    free(name);
    return decl;
}

static Decl *parse_declaration(Parser *p) {
    if (match(p, TOK_DIRECTIVE)) {
        return decl_directive(p->previous.lexeme);
    }
    if (match(p, TOK_KW_STRUCT)) {
        return parse_struct_decl(p);
    }
    if (match(p, TOK_KW_FN)) {
        return parse_function_decl(p);
    }

    parser_error(p, "Expected top-level declaration (directive, struct, or fn)");
    advance(p);
    return NULL;
}

/* =========================================================================
 * Public Interface
 * ========================================================================= */

Parser parser_init(const char *source) {
    Parser parser;
    parser.lexer = lexer_init(source);
    parser.had_error = 0;
    parser.previous = token_create(TOK_UNKNOWN, "", 0, 0);
    parser.current = lexer_next_token(&parser.lexer);
    return parser;
}

Program *parser_parse(Parser *p) {
    Program *prog = program_create();

    while (!check(p, TOK_EOF)) {
        Decl *decl = parse_declaration(p);
        if (decl) {
            program_add_decl(prog, decl);
        }
        if (p->had_error) {
            break;
        }
    }

    token_free(&p->previous);
    token_free(&p->current);

    return prog;
}
