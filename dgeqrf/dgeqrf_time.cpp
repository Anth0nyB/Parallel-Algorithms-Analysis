#include <omp.h>

#include <iostream>

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgeqrf_(LAPACK_INT *m, LAPACK_INT *n, double *A, LAPACK_INT *lda, double *tau, double *work, LAPACK_INT *lwork, LAPACK_INT *info);
}

void fillMat(double *mat, int len) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: " << argv[0] << " <m> <n>" << endl;
        return -1;
    }

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT lda = m;
    LAPACK_INT info = 0;
    LAPACK_INT lwork = -1;

    double wkopt;

    double *A = new double[lda * n];
    double *tau = new double[m > n ? m : n];
    double *work = new double[1];

    dgeqrf_(&m, &n, A, &lda, tau, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;
    delete[] work;
    work = new double[lwork];

    fillMat(A, lda * n);

    double start, end;

    start = omp_get_wtime();
    dgeqrf_(&m, &n, A, &lda, tau, work, &lwork, &info);
    end = omp_get_wtime();

    cout << end - start;

    delete[] A;
    delete[] tau;
    delete[] work;

    return (int)info;
}