CC ?= clang
CFLAGS := -O2 -g -std=c11 -march=native -Wall -Wextra -Wpedantic -Werror -Wfatal-errors

all: src/count-eq.o

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $<
