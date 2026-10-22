CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11 -D_GNU_SOURCE
SRC = compiler/src/diagnostics.c \
      compiler/src/lexer.c \
      compiler/src/ast.c \
      compiler/src/parser.c \
      compiler/src/semantics.c \
      compiler/src/x86_64.c \
      compiler/src/elf64.c \
      compiler/src/rawbin.c \
      compiler/src/disasm.c \
      compiler/src/main.c

TARGET = bin/i

all: $(TARGET) symlink

$(TARGET): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC)
	@chmod +x $@

symlink: $(TARGET)
	@ln -sf bin/i i

test: $(TARGET)
	@./bin/i test

clean:
	rm -f bin/i i /tmp/i_run_* /tmp/test*

.PHONY: all clean test symlink
