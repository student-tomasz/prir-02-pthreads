CC=gcc
CFLAGS=-Wall
LDFLAGS=-pthread

all: tmm

clean:
	rm -rf tmm

tmm: threaded_matrix_multiply.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ && ./$@ A.txt B.txt
