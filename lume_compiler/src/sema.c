/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#include "sema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *custom_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);
    if (dup) memcpy(dup, s, len);
    return dup;
}

/* =========================================================================
 * Scope & Symbol Table Operations
 * ========================================================================= */

static Scope *scope_create(Scope *parent) {
    Scope *scope = (Scope *)calloc(1, sizeof(Scope));
    scope->parent = parent;
    return scope;
}

static void scope_free(Scope *scope) {
    if (!scope) return;
    Symbol *sym = scope->symbols;
    while (sym) {
        Symbol *next = sym->next;
        free(sym->name);
        // Types are owned by AST, but deep copies would be freed here if cloned
        free(sym);
        sym = next;
    }
    free(scope);
}

static void scope_push(Sema *s) {
    s->current_scope = scope_create(s->current_scope);
}

static void scope_pop(Sema *s) {
    if (!s->current_scope) return;
    Scope *parent = s->current_scope->parent;
    scope_free(s->current_scope);
    s->current_scope = parent;
}

static Symbol *scope_lookup(Scope *scope, const char *name, SymbolKind kind) {
    for (Scope *sc = scope; sc != NULL; sc = sc->parent) {
        for (Symbol *sym = sc->symbols; sym != NULL; sym = sym->next) {
            if (sym->kind == kind && strcmp(sym->name, name) == 0) {
                return sym;
            }
        }
    }
    return NULL;
}

static Symbol *scope_lookup_current(Scope *scope, const char *name, SymbolKind kind) {
    if (!scope) return NULL;
    for (Symbol *sym = scope->symbols; sym != NULL; sym = sym->next) {
        if (sym->kind == kind && strcmp(sym->name, name) == 0) {
            return sym;
        }
    }
    return NULL;
}

static void sema_error(Sema *s, int line, int col, const char *msg) {
    s->error_count++;
    fprintf(stderr, "[Line %d, Col %d] Semantic Error: %s\n", line, col, msg);
}

/* =========================================================================
 * Symbol Registration Helpers
 * ========================================================================= */

static void add_var_symbol(Sema *s, const char *name, Type *type, int line, int col) {
    if (scope_lookup_current(s->current_scope, name, SYM_VAR)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Redeclaration of variable '%s' in the same scope", name);
        sema_error(s, line, col, buf);
        return;
    }
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->kind = SYM_VAR;
    sym->name = custom_strdup(name);
    sym->type = type;
    sym->next = s->current_scope->symbols;
    s->current_scope->symbols = sym;
}

static void add_struct_symbol(Sema *s, const char *name, StructField *fields, int field_count, int line, int col) {
    if (scope_lookup(s->global_scope, name, SYM_STRUCT)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Redefinition of struct '%s'", name);
        sema_error(s, line, col, buf);
        return;
    }
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->kind = SYM_STRUCT;
    sym->name = custom_strdup(name);
    sym->fields = fields;
    sym->field_count = field_count;
    sym->next = s->global_scope->symbols;
    s->global_scope->symbols = sym;
}

static void add_func_symbol(Sema *s, const char *name, Type *ret_type, Param *params, int param_count, int line, int col) {
    if (scope_lookup(s->global_scope, name, SYM_FUNC)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Redefinition of function '%s'", name);
        sema_error(s, line, col, buf);
        return;
    }
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->kind = SYM_FUNC;
    sym->name = custom_strdup(name);
    sym->type = ret_type;
    sym->params = params;
    sym->param_count = param_count;
    sym->next = s->global_scope->symbols;
    s->global_scope->symbols = sym;
}

/* =========================================================================
 * Expression Semantic Verification
 * ========================================================================= */

// Returns the inferred Type of an expression
static Type *check_expr(Sema *s, Expr *e) {
    if (!e) return NULL;

    switch (e->kind) {
        case EXPR_INT_LIT:
            return type_primitive(TYPE_INT);

        case EXPR_STR_LIT:
            return type_primitive(TYPE_CHAR);

        case EXPR_VAR: {
            Symbol *sym = scope_lookup(s->current_scope, e->as.var_name, SYM_VAR);
            if (!sym) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Use of undeclared variable '%s'", e->as.var_name);
                sema_error(s, e->line, e->col, buf);
                return type_primitive(TYPE_INT);
            }
            return sym->type;
        }

        case EXPR_BINARY: {
            check_expr(s, e->as.binary.left);
            check_expr(s, e->as.binary.right);
            return type_primitive(TYPE_INT); // Arithmetic and comparisons yield int
        }

        case EXPR_UNARY: {
            return check_expr(s, e->as.unary.operand);
        }

        case EXPR_CALL: {
            for (int i = 0; i < e->as.call.arg_count; i++) {
                check_expr(s, e->as.call.args[i]);
            }
            Symbol *fn = scope_lookup(s->global_scope, e->as.call.callee, SYM_FUNC);
            if (fn) {
                if (fn->param_count != e->as.call.arg_count) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Function '%s' expects %d arguments, got %d",
                             e->as.call.callee, fn->param_count, e->as.call.arg_count);
                    sema_error(s, e->line, e->col, buf);
                }
                return fn->type;
            }
            // If not found in Lume functions, treat as external C function (e.g. printf)
            return type_primitive(TYPE_INT);
        }

        case EXPR_INDEX: {
            Type *arr_type = check_expr(s, e->as.index.array);
            check_expr(s, e->as.index.index);

            if (arr_type && arr_type->kind != TYPE_ARRAY) {
                sema_error(s, e->line, e->col, "Subscripted value is not an array");
                return type_primitive(TYPE_INT);
            }
            return arr_type ? arr_type->array_elem_type : type_primitive(TYPE_INT);
        }

        case EXPR_MEMBER: {
            Type *target_type = check_expr(s, e->as.member.target);
            if (!target_type || target_type->kind != TYPE_STRUCT) {
                sema_error(s, e->line, e->col, "Member access '.' on non-struct type");
                return type_primitive(TYPE_INT);
            }

            Symbol *st = scope_lookup(s->global_scope, target_type->struct_name, SYM_STRUCT);
            if (!st) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Unknown struct '%s'", target_type->struct_name);
                sema_error(s, e->line, e->col, buf);
                return type_primitive(TYPE_INT);
            }

            for (int i = 0; i < st->field_count; i++) {
                if (strcmp(st->fields[i].name, e->as.member.member) == 0) {
                    return st->fields[i].type;
                }
            }

            char buf[128];
            snprintf(buf, sizeof(buf), "Struct '%s' has no member named '%s'",
                     target_type->struct_name, e->as.member.member);
            sema_error(s, e->line, e->col, buf);
            return type_primitive(TYPE_INT);
        }

        case EXPR_ASSIGN: {
            Type *lhs = check_expr(s, e->as.assign.lhs);
            Type *rhs = check_expr(s, e->as.assign.rhs);
            (void)rhs;
            return lhs;
        }
    }
    return NULL;
}

/* =========================================================================
 * Statement Semantic Verification
 * ========================================================================= */

static void check_stmt(Sema *s, Stmt *stmt) {
    if (!stmt) return;

    switch (stmt->kind) {
        case STMT_EXPR:
            check_expr(s, stmt->as.expr);
            break;

        case STMT_VAR_DECL:
            if (stmt->as.var_decl.init_expr) {
                check_expr(s, stmt->as.var_decl.init_expr);
            }
            add_var_symbol(s, stmt->as.var_decl.name, stmt->as.var_decl.type, 0, 0);
            break;

        case STMT_BLOCK:
            scope_push(s);
            for (int i = 0; i < stmt->as.block.stmt_count; i++) {
                check_stmt(s, stmt->as.block.stmts[i]);
            }
            scope_pop(s);
            break;

        case STMT_IF:
            check_expr(s, stmt->as.if_stmt.cond);
            check_stmt(s, stmt->as.if_stmt.then_branch);
            if (stmt->as.if_stmt.else_branch) {
                check_stmt(s, stmt->as.if_stmt.else_branch);
            }
            break;

        case STMT_WHILE:
            check_expr(s, stmt->as.while_stmt.cond);
            check_stmt(s, stmt->as.while_stmt.body);
            break;

        case STMT_RETURN:
            if (stmt->as.return_expr) {
                check_expr(s, stmt->as.return_expr);
            }
            break;
    }
}

/* =========================================================================
 * Declaration & Program Verification
 * ========================================================================= */

static void register_declaration_signatures(Sema *s, Program *prog) {
    for (int i = 0; i < prog->decl_count; i++) {
        Decl *d = prog->decls[i];
        if (d->kind == DECL_STRUCT) {
            add_struct_symbol(s, d->as.struct_decl.name,
                              d->as.struct_decl.fields,
                              d->as.struct_decl.field_count,
                              0, 0);
        } else if (d->kind == DECL_FUNCTION) {
            add_func_symbol(s, d->as.func_decl.name,
                            d->as.func_decl.return_type,
                            d->as.func_decl.params,
                            d->as.func_decl.param_count,
                            0, 0);
        }
    }
}

static void check_function_body(Sema *s, Decl *d) {
    s->current_func_ret_type = d->as.func_decl.return_type;
    scope_push(s);

    // Register function parameters into the function's scope
    for (int i = 0; i < d->as.func_decl.param_count; i++) {
        add_var_symbol(s, d->as.func_decl.params[i].name,
                          d->as.func_decl.params[i].type,
                          0, 0);
    }

    // Check statements in function body
    if (d->as.func_decl.body && d->as.func_decl.body->kind == STMT_BLOCK) {
        Stmt *b = d->as.func_decl.body;
        for (int i = 0; i < b->as.block.stmt_count; i++) {
            check_stmt(s, b->as.block.stmts[i]);
        }
    } else {
        check_stmt(s, d->as.func_decl.body);
    }

    scope_pop(s);
    s->current_func_ret_type = NULL;
}

int sema_analyze(Program *prog) {
    if (!prog) return 0;

    Sema sema;
    sema.global_scope = scope_create(NULL);
    sema.current_scope = sema.global_scope;
    sema.current_func_ret_type = NULL;
    sema.error_count = 0;

    // Pass 1: Register all struct and function signatures into the global symbol table
    register_declaration_signatures(&sema, prog);

    // Pass 2: Check function bodies and statements
    for (int i = 0; i < prog->decl_count; i++) {
        Decl *d = prog->decls[i];
        if (d->kind == DECL_FUNCTION) {
            check_function_body(&sema, d);
        }
    }

    int errors = sema.error_count;
    scope_free(sema.global_scope);
    return errors;
}
