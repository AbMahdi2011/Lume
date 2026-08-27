#ifndef LUME_TOKEN_H
#define LUME_TOKEN_H

typedef enum {
    // End of file / Special
    TOK_EOF,
    TOK_UNKNOWN,
    TOK_DIRECTIVE,   // Preprocessor pass-through lines like '#include <stdio.h>'

    // Literals & Identifiers
    TOK_IDENT,       // Variable / function / struct names
    TOK_INT_LIT,     // 123
    TOK_STR_LIT,     // "hello world"

    // Keywords
    TOK_KW_FN,       // fn
    TOK_KW_RETURN,   // return
    TOK_KW_IF,       // if
    TOK_KW_ELSE,     // else
    TOK_KW_WHILE,    // while
    TOK_KW_STRUCT,   // struct
    TOK_KW_INT,      // int
    TOK_KW_CHAR,     // char
    TOK_KW_VOID,     // void

    // Single-character symbols & operators
    TOK_PLUS,        // +
    TOK_MINUS,       // -
    TOK_STAR,        // *
    TOK_SLASH,       // /
    TOK_PERCENT,     // %
    TOK_ASSIGN,      // =
    TOK_LT,          // <
    TOK_GT,          // >
    TOK_BANG,        // !
    TOK_LPAREN,      // (
    TOK_RPAREN,      // )
    TOK_LBRACE,      // {
    TOK_RBRACE,      // }
    TOK_LBRACKET,    // [
    TOK_RBRACKET,    // ]
    TOK_SEMICOLON,   // ;
    TOK_COMMA,       // ,
    TOK_COLON,       // :
    TOK_DOT,         // .

    // Multi-character operators
    TOK_EQ_EQ,       // ==
    TOK_BANG_EQ,     // !=
    TOK_LT_EQ,       // <=
    TOK_GT_EQ,       // >=
    TOK_AND_AND,     // &&
    TOK_OR_OR        // ||
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;    // The textual representation in source
    int line;        // Line number (for error reporting)
    int col;         // Column number (for error reporting)
} Token;

// Utility functions
const char *token_type_to_string(TokenType type);
Token token_create(TokenType type, const char *lexeme, int line, int col);
void token_free(Token *tok);

#endif // LUME_TOKEN_HWE
