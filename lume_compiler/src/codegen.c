/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Helper Functions & Indentation
 * ========================================================================= */

static void emit_indent(FILE *out, int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(out, "    ");
    }
}

static void emit_type_name(FILE *out, const Type *type) {
    if (!type) {
        fprintf(out, "void");
        return;
    }
    switch (type->kind) {
        case TYPE_INT:    fprintf(out, "int"); break;
        case TYPE_CHAR:   fprintf(out, "char"); break;
        case TYPE_VOID:   fprintf(out, "void"); break;
        case TYPE_STRUCT: fprintf(out, "struct %s", type->struct_name); break;
        case TYPE_ARRAY:  emit_type_name(out, type->array_elem_type); break;
    }
}

/* =========================================================================
 * Expression Emitter
 * ========================================================================= */

static void emit_expr(FILE *out, const Expr *e) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INT_LIT:
            fprintf(out, "%d", e->as.int_val);
            break;

        case EXPR_STR_LIT:
            fprintf(out, "\"%s\"", e->as.str_val);
            break;

        case EXPR_VAR:
            fprintf(out, "%s", e->as.var_name);
            break;

        case EXPR_BINARY:
            fprintf(out, "(");
            emit_expr(out, e->as.binary.left);
            fprintf(out, " %s ", token_type_to_string(e->as.binary.op));
            emit_expr(out, e->as.binary.right);
            fprintf(out, ")");
            break;

        case EXPR_UNARY:
            fprintf(out, "(%s", token_type_to_string(e->as.unary.op));
            emit_expr(out, e->as.unary.operand);
            fprintf(out, ")");
            break;

        case EXPR_CALL:
            fprintf(out, "%s(", e->as.call.callee);
            for (int i = 0; i < e->as.call.arg_count; i++) {
                emit_expr(out, e->as.call.args[i]);
                if (i + 1 < e->as.call.arg_count) {
                    fprintf(out, ", ");
                }
            }
            fprintf(out, ")");
            break;

        case EXPR_INDEX:
            emit_expr(out, e->as.index.array);
            fprintf(out, "[");
            emit_expr(out, e->as.index.index);
            fprintf(out, "]");
            break;

        case EXPR_MEMBER:
            emit_expr(out, e->as.member.target);
            fprintf(out, ".%s", e->as.member.member);
            break;

        case EXPR_ASSIGN:
            fprintf(out, "(");
            emit_expr(out, e->as.assign.lhs);
            fprintf(out, " = ");
            emit_expr(out, e->as.assign.rhs);
            fprintf(out, ")");
            break;
    }
}

/* =========================================================================
 * Statement Emitter
 * ========================================================================= */

static void emit_stmt(FILE *out, const Stmt *s, int indent);

static void emit_block(FILE *out, const Stmt *s, int indent) {
    fprintf(out, "{\n");
    for (int i = 0; i < s->as.block.stmt_count; i++) {
        emit_stmt(out, s->as.block.stmts[i], indent + 1);
    }
    emit_indent(out, indent);
    fprintf(out, "}");
}

static void emit_stmt(FILE *out, const Stmt *s, int indent) {
    if (!s) return;

    emit_indent(out, indent);

    switch (s->kind) {
        case STMT_EXPR:
            emit_expr(out, s->as.expr);
            fprintf(out, ";\n");
            break;

        case STMT_VAR_DECL:
            emit_type_name(out, s->as.var_decl.type);
            fprintf(out, " %s", s->as.var_decl.name);
            if (s->as.var_decl.type->kind == TYPE_ARRAY) {
                fprintf(out, "[%d]", s->as.var_decl.type->array_size);
            }
            if (s->as.var_decl.init_expr) {
                fprintf(out, " = ");
                emit_expr(out, s->as.var_decl.init_expr);
            }
            fprintf(out, ";\n");
            break;

        case STMT_BLOCK:
            emit_block(out, s, indent);
            fprintf(out, "\n");
            break;

        case STMT_IF:
            fprintf(out, "if (");
            emit_expr(out, s->as.if_stmt.cond);
            fprintf(out, ") ");
            if (s->as.if_stmt.then_branch->kind == STMT_BLOCK) {
                emit_block(out, s->as.if_stmt.then_branch, indent);
            } else {
                fprintf(out, "{\n");
                emit_stmt(out, s->as.if_stmt.then_branch, indent + 1);
                emit_indent(out, indent);
                fprintf(out, "}");
            }

            if (s->as.if_stmt.else_branch) {
                fprintf(out, " else ");
                if (s->as.if_stmt.else_branch->kind == STMT_BLOCK) {
                    emit_block(out, s->as.if_stmt.else_branch, indent);
                } else {
                    fprintf(out, "{\n");
                    emit_stmt(out, s->as.if_stmt.else_branch, indent + 1);
                    emit_indent(out, indent);
                    fprintf(out, "}");
                }
            }
            fprintf(out, "\n");
            break;

        case STMT_WHILE:
            fprintf(out, "while (");
            emit_expr(out, s->as.while_stmt.cond);
            fprintf(out, ") ");
            if (s->as.while_stmt.body->kind == STMT_BLOCK) {
                emit_block(out, s->as.while_stmt.body, indent);
            } else {
                fprintf(out, "{\n");
                emit_stmt(out, s->as.while_stmt.body, indent + 1);
                emit_indent(out, indent);
                fprintf(out, "}");
            }
            fprintf(out, "\n");
            break;

        case STMT_RETURN:
            fprintf(out, "return");
            if (s->as.return_expr) {
                fprintf(out, " ");
                emit_expr(out, s->as.return_expr);
            }
            fprintf(out, ";\n");
            break;
    }
}

/* =========================================================================
 * Top-Level Declaration Emitter
 * ========================================================================= */

static void emit_struct(FILE *out, const Decl *d) {
    fprintf(out, "struct %s {\n", d->as.struct_decl.name);
    for (int i = 0; i < d->as.struct_decl.field_count; i++) {
        emit_indent(out, 1);
        Type *ft = d->as.struct_decl.fields[i].type;
        emit_type_name(out, ft);
        fprintf(out, " %s", d->as.struct_decl.fields[i].name);
        if (ft->kind == TYPE_ARRAY) {
            fprintf(out, "[%d]", ft->array_size);
        }
        fprintf(out, ";\n");
    }
    fprintf(out, "};\n\n");
}

static void emit_func_signature(FILE *out, const Decl *d) {
    emit_type_name(out, d->as.func_decl.return_type);
    fprintf(out, " %s(", d->as.func_decl.name);

    if (d->as.func_decl.param_count == 0) {
        fprintf(out, "void");
    } else {
        for (int i = 0; i < d->as.func_decl.param_count; i++) {
            Type *pt = d->as.func_decl.params[i].type;
            emit_type_name(out, pt);
            fprintf(out, " %s", d->as.func_decl.params[i].name);
            if (pt->kind == TYPE_ARRAY) {
                fprintf(out, "[%d]", pt->array_size);
            }
            if (i + 1 < d->as.func_decl.param_count) {
                fprintf(out, ", ");
            }
        }
    }
    fprintf(out, ")");
}

static void emit_function(FILE *out, const Decl *d) {
    emit_func_signature(out, d);
    fprintf(out, " ");
    if (d->as.func_decl.body->kind == STMT_BLOCK) {
        emit_block(out, d->as.func_decl.body, 0);
    } else {
        fprintf(out, "{\n");
        emit_stmt(out, d->as.func_decl.body, 1);
        fprintf(out, "}");
    }
    fprintf(out, "\n\n");
}

/* =========================================================================
 * Public Pipeline Emitter
 * ========================================================================= */

void codegen_emit(const Program *prog, FILE *out) {
    if (!prog || !out) return;

    fprintf(out, "/* Generated automatically by Lume Transpiler */\n");

    // 1. Pass: Directives (#include ...)
    for (int i = 0; i < prog->decl_count; i++) {
        if (prog->decls[i]->kind == DECL_DIRECTIVE) {
            fprintf(out, "%s\n", prog->decls[i]->as.directive_text);
        }
    }
    fprintf(out, "\n");

    // 2. Pass: Struct Definitions
    for (int i = 0; i < prog->decl_count; i++) {
        if (prog->decls[i]->kind == DECL_STRUCT) {
            emit_struct(out, prog->decls[i]);
        }
    }

    // 3. Pass: Function Forward Prototypes
    for (int i = 0; i < prog->decl_count; i++) {
        if (prog->decls[i]->kind == DECL_FUNCTION) {
            emit_func_signature(out, prog->decls[i]);
            fprintf(out, ";\n");
        }
    }
    fprintf(out, "\n");

    // 4. Pass: Function Implementations
    for (int i = 0; i < prog->decl_count; i++) {
        if (prog->decls[i]->kind == DECL_FUNCTION) {
            emit_function(out, prog->decls[i]);
        }
    }
}

int codegen_emit_to_file(const Program *prog, const char *output_filepath) {
    FILE *f = fopen(output_filepath, "w");
    if (!f) {
        perror("Failed to open output file for writing");
        return 1;
    }
    codegen_emit(prog, f);
    fclose(f);
    return 0;
}
