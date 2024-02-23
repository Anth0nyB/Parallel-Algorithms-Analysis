/*
 * Implements a naive solution to dgemm
 *
 *  Assumes TRANSA = 'N', TRANSB = 'T',
 *  ALPHA = 1, BETA = 0,
 *  LDA = m, LDB = k, LDC = m
 */

#include <omp.h>

#include <iostream>

#define LAPACK_INT long int

extern "C" void dgemm_(char *TRANSA, char *TRANSB, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *k, double *ALPHA, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *BETA, double *C, LAPACK_INT *LDC) {
#pragma omp parallel for
    for (LAPACK_INT row = 0; row < *m; ++row) {
        LAPACK_INT row_n = row * (*k);
        for (LAPACK_INT col = 0; col < *n; ++col) {
            LAPACK_INT grid = row * (*n) + col;
            LAPACK_INT col_n = col * (*k);
            for (LAPACK_INT index = 0; index < *k; ++index) {
                C[grid] += A[row_n + index] * B[col_n + index];
            }
        }
    }
}