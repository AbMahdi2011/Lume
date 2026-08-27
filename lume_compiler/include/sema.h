#ifndef LUME_SEMA_H
#define LUME_SEMA_H

#include "ast.h"

// Symbol entry types
typedef enum {
    SYM_VAR,
    SYM_FUNC,
    SYM_STRUCT
} SymbolKind;

typedef struct Symbol {
    SymbolKind kind;
    char *name;
    Type *type;                     // Type of the variable or return type of the function
    Param *params;                  // For SYM_FUNC
    int param_count;                // For SYM_FUNC
    StructField *fields;            // For SYM_STRUCT
    int field_count;                // For SYM_STRUCT
    struct Symbol *next;
} Symbol;

// Scope structure (linked list of nested scopes)
typedef struct Scope {
    Symbol *symbols;
    struct Scope *parent;
} Scope;

typedef struct {
    Scope *current_scope;
    Scope *global_scope;
    Type *current_func_ret_type;    // For checking return statements
    int error_count;
} Sema;

// Runs semantic analysis on the entire Program AST.
// Returns the number of semantic errors found (0 means success).
int sema_analyze(Program *prog);

#endif // LUME_SEMA_H
