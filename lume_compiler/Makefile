CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O2

SRC = src/token.c src/lexer.c src/ast.c src/parser.c src/sema.c src/codegen.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) examples/*.c program test_* output.c

test: $(TARGET)
	./$(TARGET) examples/test.lm
	gcc examples/test.c -o program
	./program

.PHONY: all clean test
