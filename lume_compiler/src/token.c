/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#include "token.h"
#include <stdlib.h>
#include <string.h>

// Safe string duplication helper for standard C
static char *custom_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

Token token_create(TokenType type, const char *lexeme, int line, int col) {
    Token tok;
    tok.type = type;
    tok.lexeme = custom_strdup(lexeme);
    tok.line = line;
    tok.col = col;
    return tok;
}

void token_free(Token *tok) {
    if (tok && tok->lexeme) {
        free(tok->lexeme);
        tok->lexeme = NULL;
    }
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_EOF:       return "EOF";
        case TOK_UNKNOWN:   return "UNKNOWN";
        case TOK_DIRECTIVE: return "DIRECTIVE";
        case TOK_IDENT:     return "IDENT";
        case TOK_INT_LIT:   return "INT_LIT";
        case TOK_STR_LIT:   return "STR_LIT";
        case TOK_KW_FN:     return "KW_FN";
        case TOK_KW_RETURN: return "KW_RETURN";
        case TOK_KW_IF:     return "KW_IF";
        case TOK_KW_ELSE:   return "KW_ELSE";
        case TOK_KW_WHILE:  return "KW_WHILE";
        case TOK_KW_STRUCT: return "KW_STRUCT";
        case TOK_KW_INT:    return "KW_INT";
        case TOK_KW_CHAR:   return "KW_CHAR";
        case TOK_KW_VOID:   return "KW_VOID";
        case TOK_PLUS:      return "+";
        case TOK_MINUS:     return "-";
        case TOK_STAR:      return "*";
        case TOK_SLASH:     return "/";
        case TOK_PERCENT:   return "%";
        case TOK_ASSIGN:    return "=";
        case TOK_LT:        return "<";
        case TOK_GT:        return ">";
        case TOK_BANG:      return "!";
        case TOK_LPAREN:    return "(";
        case TOK_RPAREN:    return ")";
        case TOK_LBRACE:    return "{";
        case TOK_RBRACE:    return "}";
        case TOK_LBRACKET:  return "[";
        case TOK_RBRACKET:  return "]";
        case TOK_SEMICOLON: return ";";
        case TOK_COMMA:     return ",";
        case TOK_COLON:     return ":";
        case TOK_DOT:       return ".";
        case TOK_EQ_EQ:     return "==";
        case TOK_BANG_EQ:   return "!=";
        case TOK_LT_EQ:     return "<=";
        case TOK_GT_EQ:     return ">=";
        case TOK_AND_AND:   return "&&";
        case TOK_OR_OR:     return "||";
        default:            return "INVALID";
    }
}
