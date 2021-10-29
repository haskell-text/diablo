CC ?= clang
CFLAGS := -O2 -g -std=c11 -march=native -Wall -Wextra -Wpedantic -Werror -Wfatal-errors -fsanitize=address

all: count-eq.o

clean:
	rm -rf *.o

%.o: src/%.c include/diablo.h include/utils.h
	$(CC) $(CFLAGS) -c $<
