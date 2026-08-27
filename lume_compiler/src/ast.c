#include "ast.h"
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

/* -------------------------------------------------------------------------
 * Type Constructors & Cleanup
 * ------------------------------------------------------------------------- */
Type *type_primitive(TypeKind kind) {
    Type *t = (Type *)calloc(1, sizeof(Type));
    t->kind = kind;
    return t;
}

Type *type_struct(const char *name) {
    Type *t = (Type *)calloc(1, sizeof(Type));
    t->kind = TYPE_STRUCT;
    t->struct_name = custom_strdup(name);
    return t;
}

Type *type_array(Type *elem_type, int size) {
    Type *t = (Type *)calloc(1, sizeof(Type));
    t->kind = TYPE_ARRAY;
    t->array_elem_type = elem_type;
    t->array_size = size;
    return t;
}

void type_free(Type *type) {
    if (!type) return;
    if (type->struct_name) free(type->struct_name);
    if (type->array_elem_type) type_free(type->array_elem_type);
    free(type);
}

/* -------------------------------------------------------------------------
 * Expression Constructors & Cleanup
 * ------------------------------------------------------------------------- */
Expr *expr_int_lit(int val, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_INT_LIT;
    e->line = line;
    e->col = col;
    e->as.int_val = val;
    return e;
}

Expr *expr_str_lit(const char *val, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_STR_LIT;
    e->line = line;
    e->col = col;
    e->as.str_val = custom_strdup(val);
    return e;
}

Expr *expr_var(const char *name, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_VAR;
    e->line = line;
    e->col = col;
    e->as.var_name = custom_strdup(name);
    return e;
}

Expr *expr_binary(TokenType op, Expr *left, Expr *right, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_BINARY;
    e->line = line;
    e->col = col;
    e->as.binary.op = op;
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

Expr *expr_unary(TokenType op, Expr *operand, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_UNARY;
    e->line = line;
    e->col = col;
    e->as.unary.op = op;
    e->as.unary.operand = operand;
    return e;
}

Expr *expr_call(const char *callee, Expr **args, int arg_count, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_CALL;
    e->line = line;
    e->col = col;
    e->as.call.callee = custom_strdup(callee);
    e->as.call.args = args;
    e->as.call.arg_count = arg_count;
    return e;
}

Expr *expr_index(Expr *array, Expr *index, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_INDEX;
    e->line = line;
    e->col = col;
    e->as.index.array = array;
    e->as.index.index = index;
    return e;
}

Expr *expr_member(Expr *target, const char *member, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_MEMBER;
    e->line = line;
    e->col = col;
    e->as.member.target = target;
    e->as.member.member = custom_strdup(member);
    return e;
}

Expr *expr_assign(Expr *lhs, Expr *rhs, int line, int col) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = EXPR_ASSIGN;
    e->line = line;
    e->col = col;
    e->as.assign.lhs = lhs;
    e->as.assign.rhs = rhs;
    return e;
}

void expr_free(Expr *expr) {
    if (!expr) return;
    switch (expr->kind) {
        case EXPR_INT_LIT: break;
        case EXPR_STR_LIT: free(expr->as.str_val); break;
        case EXPR_VAR:     free(expr->as.var_name); break;
        case EXPR_BINARY:
            expr_free(expr->as.binary.left);
            expr_free(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            expr_free(expr->as.unary.operand);
            break;
        case EXPR_CALL:
            free(expr->as.call.callee);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                expr_free(expr->as.call.args[i]);
            }
            if (expr->as.call.args) free(expr->as.call.args);
            break;
        case EXPR_INDEX:
            expr_free(expr->as.index.array);
            expr_free(expr->as.index.index);
            break;
        case EXPR_MEMBER:
            expr_free(expr->as.member.target);
            free(expr->as.member.member);
            break;
        case EXPR_ASSIGN:
            expr_free(expr->as.assign.lhs);
            expr_free(expr->as.assign.rhs);
            break;
    }
    free(expr);
}

/* -------------------------------------------------------------------------
 * Statement Constructors & Cleanup
 * ------------------------------------------------------------------------- */
Stmt *stmt_expr(Expr *expr) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_EXPR;
    s->as.expr = expr;
    return s;
}

Stmt *stmt_var_decl(Type *type, const char *name, Expr *init_expr) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_VAR_DECL;
    s->as.var_decl.type = type;
    s->as.var_decl.name = custom_strdup(name);
    s->as.var_decl.init_expr = init_expr;
    return s;
}

Stmt *stmt_block(Stmt **stmts, int stmt_count) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_BLOCK;
    s->as.block.stmts = stmts;
    s->as.block.stmt_count = stmt_count;
    return s;
}

Stmt *stmt_if(Expr *cond, Stmt *then_branch, Stmt *else_branch) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_IF;
    s->as.if_stmt.cond = cond;
    s->as.if_stmt.then_branch = then_branch;
    s->as.if_stmt.else_branch = else_branch;
    return s;
}

Stmt *stmt_while(Expr *cond, Stmt *body) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_WHILE;
    s->as.while_stmt.cond = cond;
    s->as.while_stmt.body = body;
    return s;
}

Stmt *stmt_return(Expr *expr) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->kind = STMT_RETURN;
    s->as.return_expr = expr;
    return s;
}

void stmt_free(Stmt *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case STMT_EXPR:
            expr_free(stmt->as.expr);
            break;
        case STMT_VAR_DECL:
            type_free(stmt->as.var_decl.type);
            free(stmt->as.var_decl.name);
            if (stmt->as.var_decl.init_expr) expr_free(stmt->as.var_decl.init_expr);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.stmt_count; i++) {
                stmt_free(stmt->as.block.stmts[i]);
            }
            if (stmt->as.block.stmts) free(stmt->as.block.stmts);
            break;
        case STMT_IF:
            expr_free(stmt->as.if_stmt.cond);
            stmt_free(stmt->as.if_stmt.then_branch);
            if (stmt->as.if_stmt.else_branch) stmt_free(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            expr_free(stmt->as.while_stmt.cond);
            stmt_free(stmt->as.while_stmt.body);
            break;
        case STMT_RETURN:
            if (stmt->as.return_expr) expr_free(stmt->as.return_expr);
            break;
    }
    free(stmt);
}

/* -------------------------------------------------------------------------
 * Declarations & Program
 * ------------------------------------------------------------------------- */
Decl *decl_directive(const char *text) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    d->kind = DECL_DIRECTIVE;
    d->as.directive_text = custom_strdup(text);
    return d;
}

Decl *decl_struct(const char *name, StructField *fields, int field_count) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    d->kind = DECL_STRUCT;
    d->as.struct_decl.name = custom_strdup(name);
    d->as.struct_decl.fields = fields;
    d->as.struct_decl.field_count = field_count;
    return d;
}

Decl *decl_function(const char *name, Param *params, int param_count, Type *return_type, Stmt *body) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    d->kind = DECL_FUNCTION;
    d->as.func_decl.name = custom_strdup(name);
    d->as.func_decl.params = params;
    d->as.func_decl.param_count = param_count;
    d->as.func_decl.return_type = return_type;
    d->as.func_decl.body = body;
    return d;
}

void decl_free(Decl *decl) {
    if (!decl) return;
    switch (decl->kind) {
        case DECL_DIRECTIVE:
            free(decl->as.directive_text);
            break;
        case DECL_STRUCT:
            free(decl->as.struct_decl.name);
            for (int i = 0; i < decl->as.struct_decl.field_count; i++) {
                type_free(decl->as.struct_decl.fields[i].type);
                free(decl->as.struct_decl.fields[i].name);
            }
            if (decl->as.struct_decl.fields) free(decl->as.struct_decl.fields);
            break;
        case DECL_FUNCTION:
            free(decl->as.func_decl.name);
            for (int i = 0; i < decl->as.func_decl.param_count; i++) {
                type_free(decl->as.func_decl.params[i].type);
                free(decl->as.func_decl.params[i].name);
            }
            if (decl->as.func_decl.params) free(decl->as.func_decl.params);
            type_free(decl->as.func_decl.return_type);
            stmt_free(decl->as.func_decl.body);
            break;
    }
    free(decl);
}

Program *program_create(void) {
    return (Program *)calloc(1, sizeof(Program));
}

void program_add_decl(Program *prog, Decl *decl) {
    prog->decls = (Decl **)realloc(prog->decls, sizeof(Decl *) * (prog->decl_count + 1));
    prog->decls[prog->decl_count++] = decl;
}

void program_free(Program *prog) {
    if (!prog) return;
    for (int i = 0; i < prog->decl_count; i++) {
        decl_free(prog->decls[i]);
    }
    if (prog->decls) free(prog->decls);
    free(prog);
}

/* -------------------------------------------------------------------------
 * AST Visualizer / Printer (For Debugging)
 * ------------------------------------------------------------------------- */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

static void print_expr(const Expr *e, int indent) {
    if (!e) return;
    print_indent(indent);
    switch (e->kind) {
        case EXPR_INT_LIT: printf("IntLiteral(%d)\n", e->as.int_val); break;
        case EXPR_STR_LIT: printf("StringLiteral(\"%s\")\n", e->as.str_val); break;
        case EXPR_VAR:     printf("Variable(%s)\n", e->as.var_name); break;
        case EXPR_BINARY:
            printf("BinaryOp(%s)\n", token_type_to_string(e->as.binary.op));
            print_expr(e->as.binary.left, indent + 1);
            print_expr(e->as.binary.right, indent + 1);
            break;
        case EXPR_UNARY:
            printf("UnaryOp(%s)\n", token_type_to_string(e->as.unary.op));
            print_expr(e->as.unary.operand, indent + 1);
            break;
        case EXPR_CALL:
            printf("Call(%s)\n", e->as.call.callee);
            for (int i = 0; i < e->as.call.arg_count; i++) {
                print_expr(e->as.call.args[i], indent + 1);
            }
            break;
        case EXPR_INDEX:
            printf("IndexAccess:\n");
            print_expr(e->as.index.array, indent + 1);
            print_expr(e->as.index.index, indent + 1);
            break;
        case EXPR_MEMBER:
            printf("MemberAccess(.%s):\n", e->as.member.member);
            print_expr(e->as.member.target, indent + 1);
            break;
        case EXPR_ASSIGN:
            printf("Assignment:\n");
            print_expr(e->as.assign.lhs, indent + 1);
            print_expr(e->as.assign.rhs, indent + 1);
            break;
    }
}

static void print_stmt(const Stmt *s, int indent) {
    if (!s) return;
    print_indent(indent);
    switch (s->kind) {
        case STMT_EXPR:
            printf("ExprStmt:\n");
            print_expr(s->as.expr, indent + 1);
            break;
        case STMT_VAR_DECL:
            printf("VarDecl(%s):\n", s->as.var_decl.name);
            if (s->as.var_decl.init_expr) {
                print_expr(s->as.var_decl.init_expr, indent + 1);
            }
            break;
        case STMT_BLOCK:
            printf("Block:\n");
            for (int i = 0; i < s->as.block.stmt_count; i++) {
                print_stmt(s->as.block.stmts[i], indent + 1);
            }
            break;
        case STMT_IF:
            printf("IfStmt:\n");
            print_indent(indent + 1); printf("Condition:\n");
            print_expr(s->as.if_stmt.cond, indent + 2);
            print_indent(indent + 1); printf("Then:\n");
            print_stmt(s->as.if_stmt.then_branch, indent + 2);
            if (s->as.if_stmt.else_branch) {
                print_indent(indent + 1); printf("Else:\n");
                print_stmt(s->as.if_stmt.else_branch, indent + 2);
            }
            break;
        case STMT_WHILE:
            printf("WhileStmt:\n");
            print_indent(indent + 1); printf("Condition:\n");
            print_expr(s->as.while_stmt.cond, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            print_stmt(s->as.while_stmt.body, indent + 2);
            break;
        case STMT_RETURN:
            printf("ReturnStmt:\n");
            if (s->as.return_expr) {
                print_expr(s->as.return_expr, indent + 1);
            }
            break;
    }
}

void program_print(const Program *prog) {
    if (!prog) return;
    printf("=== Program AST ===\n");
    for (int i = 0; i < prog->decl_count; i++) {
        Decl *d = prog->decls[i];
        switch (d->kind) {
            case DECL_DIRECTIVE:
                printf("Directive: %s\n", d->as.directive_text);
                break;
            case DECL_STRUCT:
                printf("Struct: %s (%d fields)\n", d->as.struct_decl.name, d->as.struct_decl.field_count);
                break;
            case DECL_FUNCTION:
                printf("Function: %s (%d params)\n", d->as.func_decl.name, d->as.func_decl.param_count);
                print_stmt(d->as.func_decl.body, 1);
                break;
        }
    }
}
