/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Lexer lexer_init(const char *source) {
    Lexer lexer;
    lexer.source = source;
    lexer.length = strlen(source);
    lexer.cursor = 0;
    lexer.line = 1;
    lexer.col = 1;
    return lexer;
}

static char peek(const Lexer *lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    return lexer->source[lexer->cursor];
}

static char peek_next(const Lexer *lexer) {
    if (lexer->cursor + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->cursor + 1];
}

static char advance(Lexer *lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    char c = lexer->source[lexer->cursor++];
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    while (lexer->cursor < lexer->length) {
        char c = peek(lexer);

        // Whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lexer);
        }
        // Single-line comment: //
        else if (c == '/' && peek_next(lexer) == '/') {
            while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                advance(lexer);
            }
        }
        // Multi-line comment: /* ... */
        else if (c == '/' && peek_next(lexer) == '*') {
            advance(lexer); // skip '/'
            advance(lexer); // skip '*'
            while (peek(lexer) != '\0') {
                if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                    advance(lexer); // skip '*'
                    advance(lexer); // skip '/'
                    break;
                }
                advance(lexer);
            }
        } else {
            break;
        }
    }
}

// Scans lines starting with '#' (like #include <stdio.h>)
static Token scan_directive(Lexer *lexer, int start_line, int start_col) {
    size_t start = lexer->cursor;
    while (peek(lexer) != '\n' && peek(lexer) != '\0') {
        advance(lexer);
    }
    size_t len = lexer->cursor - start;
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, &lexer->source[start], len);
    buffer[len] = '\0';

    Token tok = token_create(TOK_DIRECTIVE, buffer, start_line, start_col);
    free(buffer);
    return tok;
}

// Scans numbers: 1234
static Token scan_number(Lexer *lexer, int start_line, int start_col) {
    size_t start = lexer->cursor;
    while (isdigit((unsigned char)peek(lexer))) {
        advance(lexer);
    }
    size_t len = lexer->cursor - start;
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, &lexer->source[start], len);
    buffer[len] = '\0';

    Token tok = token_create(TOK_INT_LIT, buffer, start_line, start_col);
    free(buffer);
    return tok;
}

// Scans string literals: "hello world"
static Token scan_string(Lexer *lexer, int start_line, int start_col) {
    advance(lexer); // Skip opening quote
    size_t start = lexer->cursor;

    while (peek(lexer) != '"' && peek(lexer) != '\0') {
        if (peek(lexer) == '\\' && peek_next(lexer) != '\0') {
            advance(lexer); // skip escape char
        }
        advance(lexer);
    }

    size_t len = lexer->cursor - start;
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, &lexer->source[start], len);
    buffer[len] = '\0';

    if (peek(lexer) == '"') {
        advance(lexer); // Skip closing quote
    }

    Token tok = token_create(TOK_STR_LIT, buffer, start_line, start_col);
    free(buffer);
    return tok;
}

// Scans identifiers and keywords
static Token scan_identifier_or_keyword(Lexer *lexer, int start_line, int start_col) {
    size_t start = lexer->cursor;
    while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    size_t len = lexer->cursor - start;
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, &lexer->source[start], len);
    buffer[len] = '\0';

    TokenType type = TOK_IDENT;

    // Check keywords
    if (strcmp(buffer, "fn") == 0)          type = TOK_KW_FN;
    else if (strcmp(buffer, "return") == 0) type = TOK_KW_RETURN;
    else if (strcmp(buffer, "if") == 0)     type = TOK_KW_IF;
    else if (strcmp(buffer, "else") == 0)   type = TOK_KW_ELSE;
    else if (strcmp(buffer, "while") == 0)  type = TOK_KW_WHILE;
    else if (strcmp(buffer, "struct") == 0) type = TOK_KW_STRUCT;
    else if (strcmp(buffer, "int") == 0)    type = TOK_KW_INT;
    else if (strcmp(buffer, "char") == 0)   type = TOK_KW_CHAR;
    else if (strcmp(buffer, "void") == 0)   type = TOK_KW_VOID;

    Token tok = token_create(type, buffer, start_line, start_col);
    free(buffer);
    return tok;
}

Token lexer_next_token(Lexer *lexer) {
    skip_whitespace_and_comments(lexer);

    int start_line = lexer->line;
    int start_col = lexer->col;

    if (lexer->cursor >= lexer->length) {
        return token_create(TOK_EOF, "", start_line, start_col);
    }

    char c = peek(lexer);

    // Preprocessor directive: #include ...
    if (c == '#') {
        return scan_directive(lexer, start_line, start_col);
    }

    // Number literals
    if (isdigit((unsigned char)c)) {
        return scan_number(lexer, start_line, start_col);
    }

    // String literals
    if (c == '"') {
        return scan_string(lexer, start_line, start_col);
    }

    // Identifiers & keywords
    if (isalpha((unsigned char)c) || c == '_') {
        return scan_identifier_or_keyword(lexer, start_line, start_col);
    }

    // Multi-character & single-character operators
    advance(lexer);
    switch (c) {
        case '+': return token_create(TOK_PLUS, "+", start_line, start_col);
        case '-': return token_create(TOK_MINUS, "-", start_line, start_col);
        case '*': return token_create(TOK_STAR, "*", start_line, start_col);
        case '/': return token_create(TOK_SLASH, "/", start_line, start_col);
        case '%': return token_create(TOK_PERCENT, "%", start_line, start_col);
        case '(': return token_create(TOK_LPAREN, "(", start_line, start_col);
        case ')': return token_create(TOK_RPAREN, ")", start_line, start_col);
        case '{': return token_create(TOK_LBRACE, "{", start_line, start_col);
        case '}': return token_create(TOK_RBRACE, "}", start_line, start_col);
        case '[': return token_create(TOK_LBRACKET, "[", start_line, start_col);
        case ']': return token_create(TOK_RBRACKET, "]", start_line, start_col);
        case ';': return token_create(TOK_SEMICOLON, ";", start_line, start_col);
        case ',': return token_create(TOK_COMMA, ",", start_line, start_col);
        case ':': return token_create(TOK_COLON, ":", start_line, start_col);
        case '.': return token_create(TOK_DOT, ".", start_line, start_col);

        case '=':
            if (peek(lexer) == '=') {
                advance(lexer);
                return token_create(TOK_EQ_EQ, "==", start_line, start_col);
            }
            return token_create(TOK_ASSIGN, "=", start_line, start_col);

        case '!':
            if (peek(lexer) == '=') {
                advance(lexer);
                return token_create(TOK_BANG_EQ, "!=", start_line, start_col);
            }
            return token_create(TOK_BANG, "!", start_line, start_col);

        case '<':
            if (peek(lexer) == '=') {
                advance(lexer);
                return token_create(TOK_LT_EQ, "<=", start_line, start_col);
            }
            return token_create(TOK_LT, "<", start_line, start_col);

        case '>':
            if (peek(lexer) == '=') {
                advance(lexer);
                return token_create(TOK_GT_EQ, ">=", start_line, start_col);
            }
            return token_create(TOK_GT, ">", start_line, start_col);

        case '&':
            if (peek(lexer) == '&') {
                advance(lexer);
                return token_create(TOK_AND_AND, "&&", start_line, start_col);
            }
            break;

        case '|':
            if (peek(lexer) == '|') {
                advance(lexer);
                return token_create(TOK_OR_OR, "||", start_line, start_col);
            }
            break;
    }

    char unknown_str[2] = { c, '\0' };
    return token_create(TOK_UNKNOWN, unknown_str, start_line, start_col);
}
