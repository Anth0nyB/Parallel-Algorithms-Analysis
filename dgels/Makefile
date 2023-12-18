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

dgels: time perf

time: dgels_time.cpp
	${CC} -o dgels_def_t ${flags} dgels_time.cpp ${liblapack_def} ${REFBLAS}
	${CC} -o dgels_ob_t ${flags} dgels_time.cpp ${liblapack_ob} ${OPENBLAS}
	${CC} -o dgels_mkl_t ${flags} dgels_time.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl}

perf: dgels_perf.cpp
	${CC} -o dgels_def_p ${flags} ${papiflags} dgels_perf.cpp ${liblapack_def} ${REFBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgels_ob_p ${flags} ${papiflags} dgels_perf.cpp ${liblapack_ob} ${OPENBLAS} -I${PAPI_INCLUDE} -L${PAPI_LIB}
	${CC} -o dgels_mkl_p ${flags} ${papiflags} dgels_perf.cpp -L ${mklflags} -L${MKL_LIB} ${liblapack_mkl} -I${PAPI_INCLUDE} -L${PAPI_LIB}

clean:
	rm dgels_def_t dgels_ob_t dgels_mkl_t dgels_def_p dgels_ob_p dgels_mkl_p

