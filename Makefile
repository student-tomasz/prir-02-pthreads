CC=gcc
CFLAGS=-Wall
LDFLAGS=

all: tmmul

mmul: matrix_mul.c
	$(CC) $(CFLAGS) $< -o $@ && ./$@

tmmul: threaded_matrix_mul.c
	$(CC) $(CFLAGS) $< -o $@ && ./$@

clean:
	rm -rf tmmul mmul
