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

dgeqrf: time perf

time: dgeqrf_time.cpp
	${CC} -o dgeqrf_def_t ${flags} dgeqrf_time.cpp ${liblapack_def} ${REFBLAS}
	${CC} -o dgeqrf_ob_t ${flags} dgeqrf_time.cpp ${liblapack_ob} ${OPENBLAS}
	${CC} -o dgeqrf_mkl_t ${flags} dgeqrf_time.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl}

perf: dgeqrf_perf.cpp
	${CC} -o dgeqrf_def_p ${flags} ${papiflags} dgeqrf_perf.cpp ${liblapack_def} ${REFBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgeqrf_ob_p ${flags} ${papiflags} dgeqrf_perf.cpp ${liblapack_ob} ${OPENBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgeqrf_mkl_p ${flags} ${papiflags} dgeqrf_perf.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl} -I${PAPI_INCLUDE} -L${PAPI_LIB}

clean:
	rm dgeqrf_def_t dgeqrf_ob_t dgeqrf_mkl_t dgeqrf_def_p dgeqrf_ob_p dgeqrf_mkl_p

