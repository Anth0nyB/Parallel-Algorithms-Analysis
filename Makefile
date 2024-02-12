.PHONY: all dgels dgemm dgeqrf dsymm

all: dgels dgemm dgeqrf

dgels:
	cd dgels && make all

dgemm:
	cd dgemm && make all

dgeqrf:
	cd dgeqrf && make all

dsymm:
	cd dsymm && make all

clean:
	cd dgels && make clean
	cd dgemm && make clean
	cd dgeqrf && make clean
	cd dsymm && make clean