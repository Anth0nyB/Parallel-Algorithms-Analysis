CC = g++

liblapack_def = ../libs/LAPACK_Def/liblapack.a
liblapack_ob = ../libs/LAPACK_OB/liblapack.a
liblapack_mkl = ../libs/LAPACK_MKL/liblapack.a

REFBLAS = ../libs/LAPACK_Def/librefblas.a
OPENBLAS = ../libs/OpenBLAS-0.3.24/libopenblas_skylakexp-r0.3.24.a

PAPI_INCLUDE = ../libs/papi-7.0.1/include
PAPI_LIB = ../libs/papi-7.0.1/lib
MKL_LIB = ${MKLROOT}/lib/intel64

flags = -lgfortran -lpthread -lgomp -fopenmp
papiflags = -lpapi -lpfm
mklflags = libmkl_blas95_ilp64.a -lmkl_gf_ilp64 -lmkl_gnu_thread -lmkl_core


all: ob mkl def

ob: dgeqrf.cpp
	${CC} -o dgeqrf_ob ${flags} ${papiflags} ../perf.cpp dgeqrf.cpp ${liblapack_ob} ${OPENBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}

mkl: dgeqrf.cpp
	${CC} -o dgeqrf_mkl ${flags} ${papiflags} ../perf.cpp dgeqrf.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl} -I${PAPI_INCLUDE} -L${PAPI_LIB}

def: dgeqrf.cpp
	${CC} -o dgeqrf_def ${flags} ${papiflags} ../perf.cpp dgeqrf.cpp ${liblapack_def} ${REFBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}

clean:
	rm dgeqrf_ob dgeqrf_mkl dgeqrf_def
