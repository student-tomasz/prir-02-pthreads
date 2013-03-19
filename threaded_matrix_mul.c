#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_COUNT 3

double sum = 0.0;

void multiply(double **A, int m_a, int n_a, double **B, int m_b, int n_b, double **C)
{
    int i, j, k;
    double s;

    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            s = 0;
            for(k = 0; k < n_a; k++) {
                s += A[i][k] * B[k][j];
            }
            C[i][j] = s;
        }
    }
}

typedef struct _tm_arg_t {
    int start_row;
    int finish_row;
    double **A;
    int n_a;
    double **B;
    int n_b;
    double **C;

    double *sum;
    pthread_mutex_t *sum_mutex;
} tm_arg_t;

void *threaded_multiply(void *args)
{
    tm_arg_t *arg = (tm_arg_t *)args;

    double partial_sum;
    int i, j, k;
    for (i = arg->start_row; i < arg->finish_row; i++) {
        for (j = 0; j < arg->n_b; j++) {
            partial_sum = 0.0;
            for(k = 0; k < arg->n_a; k++) {
                partial_sum += arg->A[i][k] * arg->B[k][j];
            }
            arg->C[i][j] = partial_sum;
        }
    }

    double rows_sum = 0.0, row_sum;
    for (i = arg->start_row; i < arg->finish_row; i++) {
        row_sum = 0.0;
        for (j = 0; j < arg->n_b; j++) {
            row_sum += arg->C[i][j];
        }
        rows_sum += row_sum;
    }
    pthread_mutex_lock(arg->sum_mutex);
    sum += rows_sum;
    pthread_mutex_unlock(arg->sum_mutex);

    pthread_exit(NULL);
}

void print_matrix(double **A, int m, int n)
{
    int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", A[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    FILE *fpa,
         *fpb;
    double **A, **B, **C;
    int m_a, n_a, m_b, n_b, m_c, n_c;
    int i, j, k;
    double x;

    fpa = fopen("A.txt", "r");
    fpb = fopen("B.txt", "r");
    if (fpa == NULL || fpb == NULL) {
        perror("blad otwarcia pliku\n");
        exit(EXIT_FAILURE);
    }

    fscanf (fpa, "%d", &m_a);
    fscanf (fpa, "%d", &n_a);
    fscanf (fpb, "%d", &m_b);
    fscanf (fpb, "%d", &n_b);
    printf("macierz %s ma wymiar %dx%d\n", "A", m_a, n_a);
    printf("macierz %s ma wymiar %dx%d\n", "B", m_b, n_b);
    m_c = m_a;
    n_c = n_b;
    printf("macierz %s ma wymiar %dx%d\n", "C", m_c, n_c);

    if (n_a != m_b) {
        printf("macierzy nie mozna mnozyc\n");
        exit(EXIT_FAILURE);
    }

    A = malloc(sizeof(*A) * m_a);
    for (i = 0; i < m_a; i++) {
        A[i] = malloc(sizeof(**A) * n_a);
    }

    B = malloc(sizeof(*B) * m_b);
    for (i = 0; i < m_b; i++) {
        B[i] = malloc(sizeof(**B) * n_b);
    }

    C = malloc(sizeof(*C) * m_c);
    for (i = 0; i < m_c; i++) {
        C[i] = malloc(sizeof(**C) * n_b);
    }


    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_a; j++) {
            fscanf(fpa, "%lf", &x);
            A[i][j] = x;
        }
    }
    printf("A:\n");
    print_matrix(A, m_a, n_a);

    for (i = 0; i < m_b; i++) {
        for (j = 0; j < n_b; j++) {
            fscanf(fpb, "%lf", &x);
            B[i][j] = x;
        }
    }
    printf("B:\n");
    print_matrix(B, m_b, n_b);


    multiply(A, m_a, n_a, B, m_b, n_b, C);
    printf("C:\n");
    print_matrix(C, m_c, n_c);


    sum = 0.0;
    pthread_mutex_t sum_mutex;
    if (pthread_mutex_init(&sum_mutex, NULL) != 0) {
        fprintf(stderr, "blad tworzenia mutexa\n");
        exit(EXIT_FAILURE);
    }

    int start_rows[THREAD_COUNT];
    int finish_rows[THREAD_COUNT];
    int full_rows = m_c / THREAD_COUNT;
    for (i = 0; i < THREAD_COUNT-1; i++) {
        start_rows[i] = i*full_rows;
        finish_rows[i] = start_rows[i] + full_rows;
    }
    start_rows[THREAD_COUNT-1] = m_c - (full_rows*THREAD_COUNT);
    finish_rows[THREAD_COUNT-1] = m_c;

    pthread_t *threads = malloc(sizeof(*threads) * THREAD_COUNT);
    tm_arg_t *args = malloc(sizeof(*args) * THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT; i++) {
        args[i].start_row = start_rows[i];
        args[i].finish_row = finish_rows[i];
        args[i].A = A;
        args[i].n_a = n_a;
        args[i].B = B;
        args[i].n_b = n_b;
        args[i].C = C;
        args[i].sum = &sum;
        args[i].sum_mutex = &sum_mutex;
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(threads + i, NULL, threaded_multiply, (void *)(args + i));
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    if (pthread_mutex_destroy(&sum_mutex) != 0) {
        fprintf(stderr, "blad zniszczenia mutexa\n");
        exit(EXIT_FAILURE);
    }

    printf("C:\n");
    print_matrix(C, m_c, n_c);
    printf("sum = %lf\n", sum);

    free(args);
    free(threads);

    for (i = 0; i < m_c; i++) {
        free(C[i]);
    }
    free(C);

    for (i = 0; i < m_b; i++) {
        free(B[i]);
    }
    free(B);

    for (i = 0; i < m_a; i++) {
        free(A[i]);
    }
    free(A);

    fclose(fpa);
    fclose(fpb);

    return 0;
}
