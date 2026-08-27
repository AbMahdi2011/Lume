/* Copyright (C) 2026 Abdullah Al Mahdi

Lume is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "sema.h"
#include "codegen.h"

// Helper to read an entire file into memory
static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory reading '%s'\n", path);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), size, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

// Generates an output .c filename from the input path (e.g. test.lm -> test.c)
static void get_output_path(const char *input_path, char *out_buffer, size_t max_len) {
    strncpy(out_buffer, input_path, max_len - 1);
    out_buffer[max_len - 1] = '\0';

    char *dot = strrchr(out_buffer, '.');
    if (dot) {
        *dot = '\0';
    }
    strncat(out_buffer, ".c", max_len - strlen(out_buffer) - 1);
}

int main(int argc, char *argv[]) {
    char input_filepath[256];

    printf("========================================\n");
    printf("         Lume Language Transpiler       \n");
    printf("========================================\n\n");

    // If passed via command line, use it; otherwise ask interactively
    if (argc > 1) {
        strncpy(input_filepath, argv[1], sizeof(input_filepath) - 1);
        input_filepath[sizeof(input_filepath) - 1] = '\0';
    } else {
        printf("Enter source file name (e.g., examples/test.lm): ");
        if (!fgets(input_filepath, sizeof(input_filepath), stdin)) {
            fprintf(stderr, "Error reading input\n");
            return 1;
        }

        // Strip trailing newline character
        size_t len = strlen(input_filepath);
        if (len > 0 && input_filepath[len - 1] == '\n') {
            input_filepath[len - 1] = '\0';
        }
    }

    if (strlen(input_filepath) == 0) {
        fprintf(stderr, "Error: No file specified.\n");
        return 1;
    }

    printf("\n[1/4] Reading '%s'...\n", input_filepath);
    char *source = read_file(input_filepath);
    if (!source) {
        return 1;
    }

    printf("[2/4] Lexing and Parsing AST...\n");
    Parser parser = parser_init(source);
    Program *prog = parser_parse(&parser);

    if (parser.had_error) {
        fprintf(stderr, "\n[FAILED] Compilation aborted due to syntax errors.\n");
        program_free(prog);
        free(source);
        return 1;
    }

    printf("[3/4] Running Semantic Analysis...\n");
    int sema_errors = sema_analyze(prog);
    if (sema_errors > 0) {
        fprintf(stderr, "\n[FAILED] Compilation aborted due to %d semantic error(s).\n", sema_errors);
        program_free(prog);
        free(source);
        return 1;
    }

    char output_filepath[300];
    get_output_path(input_filepath, output_filepath, sizeof(output_filepath));

    printf("[4/4] Emitting C code to '%s'...\n", output_filepath);
    if (codegen_emit_to_file(prog, output_filepath) != 0) {
        program_free(prog);
        free(source);
        return 1;
    }

    printf("\n[SUCCESS] Transpilation completed successfully!\n");
    printf("Generated file: %s\n\n", output_filepath);
    printf("To compile and run your program:\n");
    printf("  gcc %s -o program && ./program\n\n", output_filepath);

    program_free(prog);
    free(source);
    return 0;
}
