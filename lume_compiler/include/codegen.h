#ifndef LUME_CODEGEN_H
#define LUME_CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Emits clean C source code for the given Program AST to a stream
void codegen_emit(const Program *prog, FILE *out);

// Convenience function to write emitted C code to a destination file path
int codegen_emit_to_file(const Program *prog, const char *output_filepath);

#endif // LUME_CODEGEN_H
