CC = g++

liblapack_def = ../libs/LAPACK_Def/liblapack.a
liblapack_ob = ../libs/LAPACK_OB/liblapack.a
liblapack_mkl = ../libs/LAPACK_MKL/liblapack.a

REFBLAS = ../libs/LAPACK_Def/librefblas.a
OPENBLAS = ../libs/OpenBLAS-0.3.24/libopenblas_skylakexp-r0.3.24.a

PAPI_INCLUDE = ../libs/papi-7.0.1/include
PAPI_LIB = ../libs/papi-7.0.1/lib
MKL_LIB = ${MKLROOT}/lib/intel64

flags = -lgfortran -lpthread -lgomp
papiflags = -lpapi -lpfm
mklflags = libmkl_blas95_ilp64.a -lmkl_gf_ilp64 -lmkl_gnu_thread -lmkl_core


all: time perf

dgemm: time perf

time: dgemm_time.cpp
	${CC} -o dgemm_def_t ${flags} dgemm_time.cpp ${liblapack_def} ${REFBLAS}
	${CC} -o dgemm_ob_t ${flags} dgemm_time.cpp ${liblapack_ob} ${OPENBLAS}
	${CC} -o dgemm_mkl_t ${flags} dgemm_time.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl}

perf: dgemm_perf.cpp
	${CC} -o dgemm_def_p ${flags} ${papiflags} dgemm_perf.cpp ${liblapack_def} ${REFBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgemm_ob_p ${flags} ${papiflags} dgemm_perf.cpp ${liblapack_ob} ${OPENBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgemm_mkl_p ${flags} ${papiflags} dgemm_perf.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl} -I${PAPI_INCLUDE} -L${PAPI_LIB}

clean:
	rm dgemm_def_t dgemm_ob_t dgemm_mkl_t dgemm_def_p dgemm_ob_p dgemm_mkl_p

