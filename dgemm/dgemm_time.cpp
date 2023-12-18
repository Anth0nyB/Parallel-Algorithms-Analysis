#include <omp.h>

#include <iostream>

#define LAPACK_INT long int

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
void dgemm_(char *TRANSA, char *TRANSB, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *k, double *ALPHA, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *BETA, double *C, LAPACK_INT *LDC);

#ifdef __cplusplus
}
#endif

void fillMat(double *mat, int len, int fill = 1) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i * fill);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <k> <n>" << endl;
        return -1;
    }

    char Nchar = 'N';

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT k = atoi(argv[2]);
    LAPACK_INT n = atoi(argv[3]);

    double alpha = 1;
    double beta = 0;

    LAPACK_INT lda = m;
    LAPACK_INT ldb = k;
    LAPACK_INT ldc = m;

    double *A = new double[m * k];
    double *B = new double[k * n];
    double *C = new double[m * n];

    fillMat(A, m * k);
    fillMat(B, k * n);
    fillMat(C, m * n, 0);

    double start, end;

    start = omp_get_wtime();
    dgemm_(&Nchar, &Nchar, &m, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);
    end = omp_get_wtime();

    cout << end - start;

    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}