/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

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
