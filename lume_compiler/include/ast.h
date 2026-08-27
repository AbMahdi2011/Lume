/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#ifndef LUME_AST_H
#define LUME_AST_H

#include "token.h"

/* -------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------- */
typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_STRUCT,
    TYPE_ARRAY
} TypeKind;

typedef struct Type {
    TypeKind kind;
    char *struct_name;          // For TYPE_STRUCT (e.g., "Point")
    struct Type *array_elem_type; // For TYPE_ARRAY
    int array_size;             // For TYPE_ARRAY (e.g., 5 in int[5])
} Type;

Type *type_primitive(TypeKind kind);
Type *type_struct(const char *name);
Type *type_array(Type *elem_type, int size);
void type_free(Type *type);

/* -------------------------------------------------------------------------
 * Expressions
 * ------------------------------------------------------------------------- */
typedef enum {
    EXPR_INT_LIT,
    EXPR_STR_LIT,
    EXPR_VAR,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_MEMBER,
    EXPR_ASSIGN
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    int line;
    int col;
    union {
        int int_val;            // EXPR_INT_LIT
        char *str_val;          // EXPR_STR_LIT
        char *var_name;         // EXPR_VAR
        struct {
            TokenType op;
            struct Expr *left;
            struct Expr *right;
        } binary;               // EXPR_BINARY
        struct {
            TokenType op;
            struct Expr *operand;
        } unary;                // EXPR_UNARY
        struct {
            char *callee;
            struct Expr **args;
            int arg_count;
        } call;                 // EXPR_CALL
        struct {
            struct Expr *array;
            struct Expr *index;
        } index;                // EXPR_INDEX
        struct {
            struct Expr *target;
            char *member;
        } member;               // EXPR_MEMBER (e.g., point.x)
        struct {
            struct Expr *lhs;
            struct Expr *rhs;
        } assign;               // EXPR_ASSIGN (e.g., x = 5, arr[0] = 10)
    } as;
} Expr;

Expr *expr_int_lit(int val, int line, int col);
Expr *expr_str_lit(const char *val, int line, int col);
Expr *expr_var(const char *name, int line, int col);
Expr *expr_binary(TokenType op, Expr *left, Expr *right, int line, int col);
Expr *expr_unary(TokenType op, Expr *operand, int line, int col);
Expr *expr_call(const char *callee, Expr **args, int arg_count, int line, int col);
Expr *expr_index(Expr *array, Expr *index, int line, int col);
Expr *expr_member(Expr *target, const char *member, int line, int col);
Expr *expr_assign(Expr *lhs, Expr *rhs, int line, int col);
void expr_free(Expr *expr);

/* -------------------------------------------------------------------------
 * Statements
 * ------------------------------------------------------------------------- */
typedef enum {
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN
} StmtKind;

typedef struct Stmt {
    StmtKind kind;
    union {
        Expr *expr;             // STMT_EXPR
        struct {
            Type *type;
            char *name;
            Expr *init_expr;    // Optional, can be NULL
        } var_decl;             // STMT_VAR_DECL
        struct {
            struct Stmt **stmts;
            int stmt_count;
        } block;                // STMT_BLOCK ({ ... })
        struct {
            Expr *cond;
            struct Stmt *then_branch;
            struct Stmt *else_branch; // Optional, can be NULL
        } if_stmt;              // STMT_IF
        struct {
            Expr *cond;
            struct Stmt *body;
        } while_stmt;           // STMT_WHILE
        Expr *return_expr;      // STMT_RETURN (can be NULL for void)
    } as;
} Stmt;

Stmt *stmt_expr(Expr *expr);
Stmt *stmt_var_decl(Type *type, const char *name, Expr *init_expr);
Stmt *stmt_block(Stmt **stmts, int stmt_count);
Stmt *stmt_if(Expr *cond, Stmt *then_branch, Stmt *else_branch);
Stmt *stmt_while(Expr *cond, Stmt *body);
Stmt *stmt_return(Expr *expr);
void stmt_free(Stmt *stmt);

/* -------------------------------------------------------------------------
 * Declarations & Program
 * ------------------------------------------------------------------------- */
typedef struct {
    Type *type;
    char *name;
} Param;

typedef struct {
    Type *type;
    char *name;
} StructField;

typedef enum {
    DECL_DIRECTIVE,
    DECL_STRUCT,
    DECL_FUNCTION
} DeclKind;

typedef struct Decl {
    DeclKind kind;
    union {
        char *directive_text;   // DECL_DIRECTIVE (e.g., "#include <stdio.h>")
        struct {
            char *name;
            StructField *fields;
            int field_count;
        } struct_decl;          // DECL_STRUCT
        struct {
            char *name;
            Param *params;
            int param_count;
            Type *return_type;
            Stmt *body;
        } func_decl;            // DECL_FUNCTION
    } as;
} Decl;

Decl *decl_directive(const char *text);
Decl *decl_struct(const char *name, StructField *fields, int field_count);
Decl *decl_function(const char *name, Param *params, int param_count, Type *return_type, Stmt *body);
void decl_free(Decl *decl);

typedef struct {
    Decl **decls;
    int decl_count;
} Program;

Program *program_create(void);
void program_add_decl(Program *prog, Decl *decl);
void program_free(Program *prog);
void program_print(const Program *prog); // For AST visualization

#endif // LUME_AST_H
