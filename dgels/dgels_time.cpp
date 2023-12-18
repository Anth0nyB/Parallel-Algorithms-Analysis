#include <omp.h>

#include <iostream>

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgels_(char *TRANSA, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *NRHS, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *WORK, LAPACK_INT *LWORK, LAPACK_INT *INFO);
}

void fillMat(double *mat, int len) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <n> <NHRS>" << endl;
        return -1;
    }

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT NRHS = atoi(argv[3]);
    LAPACK_INT lda = m;
    LAPACK_INT ldb = m > n ? m : n;
    LAPACK_INT info = 0;
    LAPACK_INT lwork = -1;

    char Nchar = 'N';

    double wkopt;

    double *work = new double[1];

    double *A = new double[lda * n];
    double *B = new double[ldb * NRHS];

    dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;
    delete[] work;
    work = new double[lwork];

    fillMat(A, lda * n);
    fillMat(B, ldb * NRHS);

    double start, end;

    start = omp_get_wtime();
    dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, work, &lwork, &info);
    end = omp_get_wtime();

    cout << end - start;

    delete[] A;
    delete[] B;
    delete[] work;

    return (int)info;
}
