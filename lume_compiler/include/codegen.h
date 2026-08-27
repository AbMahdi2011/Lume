/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#ifndef LUME_CODEGEN_H
#define LUME_CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Emits clean C source code for the given Program AST to a stream
void codegen_emit(const Program *prog, FILE *out);

// Convenience function to write emitted C code to a destination file path
int codegen_emit_to_file(const Program *prog, const char *output_filepath);

#endif // LUME_CODEGEN_H
