#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREADS_COUNT 2

/* Added as a part of the exercise. Otherwise it wouldn't be here. */
double sum = 0.0;
pthread_mutex_t sum_mutex;

typedef struct _matrix_t {
    double **M;
    int m, n;
} matrix_t;

void read_input_matrix(const char *filename, matrix_t **A);
void malloc_output_matrix(const matrix_t *A, const matrix_t *B, matrix_t **C);
void print_matrix(const matrix_t *A);

/* Struct used for passing arguments to threads. */
typedef struct _tm_arg_t {
    const matrix_t *A, *B;
    matrix_t *C;
    int start_row, finish_row;
} tm_arg_t;

void nonthreaded_multiply(const matrix_t *A, const matrix_t *B, matrix_t *C);
void *threaded_multiply(void *args);
