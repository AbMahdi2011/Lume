#ifndef LUME_PARSER_H
#define LUME_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int had_error;
} Parser;

// Initializes the parser with source code
Parser parser_init(const char *source);

// Parses the entire source code into a Program AST
Program *parser_parse(Parser *parser);

#endif // LUME_PARSER_H
