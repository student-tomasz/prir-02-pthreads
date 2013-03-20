#include "threaded_matrix_multiply.h"

extern double sum;
extern pthread_mutex_t sum_mutex;

void read_input_matrix(const char *filename, matrix_t **A)
{
    FILE *file;
    int i, j;
    double x;

    *A = malloc(sizeof(**A));

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "blad przy otwieraniu pliku %s\n", filename);
        free(*A);
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n%d", &(*A)->m, &((*A)->n));

    (*A)->M = malloc(sizeof(*(*A)->M) * (*A)->m);
    for (i = 0; i < (*A)->m; i++) {
        (*A)->M[i] = malloc(sizeof(**(*A)->M) * (*A)->n);
    }

    for (i = 0; i < (*A)->m; i++) {
        for (j = 0; j < (*A)->n; j++) {
            fscanf(file, "%lf", &x);
            (*A)->M[i][j] = x;
        }
    }

    fclose(file);
}

void malloc_output_matrix(const matrix_t *A, const matrix_t *B, matrix_t **C)
{
    if (A->n != B->m) {
        printf("macierzy nie mozna mnozyc\n");
        exit(EXIT_FAILURE);
    }
    int i;

    *C = malloc(sizeof(**C));
    (*C)->m = A->m;
    (*C)->n = B->n;
    (*C)->M = malloc(sizeof(*(*C)->M) * (*C)->m);
    for (i = 0; i < (*C)->m; i++) {
        ((*C)->M)[i] = malloc(sizeof(**(*C)->M) * (*C)->n);
    }
}

void free_matrix(matrix_t *A)
{
    int i;
    for (i = 0; i < A->m; i++) {
        free(A->M[i]);
    }
    free(A->M);
    free(A);
}

void print_matrix(const matrix_t *A)
{
    int i, j;
    for (i = 0; i < A->m; i++) {
        for (j = 0; j < A->n; j++) {
            printf("%f ", A->M[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void create_threaded_multiply_args(tm_arg_t *args, const matrix_t *A, const matrix_t *B, matrix_t *C)
{
    int *rows_ranges, rows_per_thread;
    int i;

    rows_ranges = malloc(sizeof(*rows_ranges) * THREADS_COUNT + 1);
    rows_per_thread = C->m / THREADS_COUNT;
    for (i = 0; i < THREADS_COUNT; i++) {
        rows_ranges[i] = i * rows_per_thread;
    }
    rows_ranges[THREADS_COUNT] = C->m;

    for (i = 0; i < THREADS_COUNT; i++) {
        args[i].A = A;
        args[i].B = B;
        args[i].C = C;
        args[i].start_row = rows_ranges[i];
        args[i].finish_row = rows_ranges[i+1];
        // printf("[%d].start_row = %d\n", i, rows_ranges[i]);
        // printf("[%d].finish_row = %d\n", i, rows_ranges[i+1]);
    }

    free(rows_ranges);
}

void nonthreaded_multiply(const matrix_t *A, const matrix_t *B, matrix_t *C)
{
    int i, j, k;
    double cell_sum;

    for (i = 0; i < A->m; i++) {
        for (j = 0; j < B->n; j++) {
            cell_sum = 0.0;
            for(k = 0; k < A->n; k++) {
                cell_sum += A->M[i][k] * B->M[k][j];
            }
            C->M[i][j] = cell_sum;
        }
    }
}

void *threaded_multiply(void *args)
{
    tm_arg_t *arg = (tm_arg_t *)args;

    int i, j, k;
    double cell_sum, partial_sum = 0.0;

    for (i = arg->start_row; i < arg->finish_row; i++) {
        for (j = 0; j < arg->B->n; j++) {
            cell_sum = 0.0;
            for(k = 0; k < arg->A->n; k++) {
                cell_sum += arg->A->M[i][k] * arg->B->M[k][j];
            }
            arg->C->M[i][j] = cell_sum;
            partial_sum += cell_sum;
        }
    }

    pthread_mutex_lock(&sum_mutex);
    sum += partial_sum;
    pthread_mutex_unlock(&sum_mutex);

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    matrix_t *A, *B, *C;
    int i;

    read_input_matrix(argv[1], &A);
    print_matrix(A);
    read_input_matrix(argv[2], &B);
    print_matrix(B);
    malloc_output_matrix(A, B, &C);


    nonthreaded_multiply(A, B, C);
    print_matrix(C);
    free_matrix(C);
    malloc_output_matrix(A, B, &C);


    sum = 0.0;
    if (pthread_mutex_init(&sum_mutex, NULL) != 0) {
        fprintf(stderr, "blad tworzenia mutexa\n");
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(sizeof(*threads) * THREADS_COUNT);
    tm_arg_t *args = malloc(sizeof(*args) * THREADS_COUNT);

    create_threaded_multiply_args(args, A, B, C);

    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_create(threads + i, NULL, threaded_multiply, (void *)(args + i));
    }

    for (i = 0; i < THREADS_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    if (pthread_mutex_destroy(&sum_mutex) != 0) {
        fprintf(stderr, "blad zniszczenia mutexa\n");
        exit(EXIT_FAILURE);
    }

    print_matrix(C);
    printf("sum = %lf\n", sum);

    free(args);
    free(threads);
    free_matrix(C);
    free_matrix(B);
    free_matrix(A);

    return 0;
}
