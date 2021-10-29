CC ?= clang
CFLAGS := -O2 -g -std=c11 -march=native -Wall -Wextra -Wpedantic -Werror -Wfatal-errors
LDLIB := -ltheft

all: count-eq.o

test: test-count-eq
	./test-count-eq

clean:
	rm -rf test-*
	rm -rf *.o

test-count-eq: test/test-count-eq.c src/count-eq.c
	$(CC) $(CFLAGS) $(LDLIB) -o $@ $^

%.o: src/%.c include/diablo.h include/utils.h
	$(CC) $(CFLAGS) -c $<
