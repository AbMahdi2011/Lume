#ifndef LUME_LEXER_H
#define LUME_LEXER_H

#include "token.h"
#include <stddef.h>

typedef struct {
    const char *source;
    size_t length;
    size_t cursor;
    int line;
    int col;
} Lexer;

// Initializes the lexer with source code string
Lexer lexer_init(const char *source);

// Scans and returns the next token
Token lexer_next_token(Lexer *lexer);

#endif // LUME_LEXER_H
