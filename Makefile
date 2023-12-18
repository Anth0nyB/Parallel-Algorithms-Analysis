.PHONY: all dgels dgemm dgeqrf

all: dgels dgemm dgeqrf

dgels:
	cd dgels && make all

dgemm:
	cd dgemm && make all

dgeqrf:
	cd dgeqrf && make all

clean:
	cd dgels && make clean
	cd dgemm && make clean
	cd dgeqrf && make clean
	

