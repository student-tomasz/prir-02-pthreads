#include <stdio.h>
#include <stdlib.h>

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

void print_matrix(double **A, int m, int n)
{
    int i, j;
    printf("[");
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", A[i][j]);
        }
        printf("\n");
    }
    printf("]\n");
}

int main(int argc, char *argv[])
{
    FILE *fpa,
         *fpb;
    double **A, **B, **C;
    int m_a, n_a, m_b, n_b, m_c, n_c;
    int i, j;
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
