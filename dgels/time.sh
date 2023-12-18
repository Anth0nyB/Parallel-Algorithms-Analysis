#!/bin/bash 
# 
#SBATCH --job-name=dgels_t
#SBATCH --output=dgels_t.csv
# 
#SBATCH --partition=cmp
#SBATCH --nodes=1 
#SBATCH --ntasks=1 
#SBATCH --cpus-per-task=96
#SBATCH --mem-per-cpu=4G
#SBATCH --time=1-00:00:00 
#
#SBATCH --mail-user=abryson@scu.edu
#SBATCH --mail-type=END


# Load the modules and environment variables
source ../environment.env

# Compile the program
make all > /dev/null

# Run the Program
echo "OpenBLAS,"
echo "time,m,n,NRHS,threads,"
for m in 4000 12000 20000 30000; do
    for n in 4000 12000 20000 30000; do 
        for NRHS in 1 ; do 
            for threads in 4 12 24 48 72 96 ; do
                export OMP_NUM_THREADS=${threads}
                ./dgels_ob_t ${m} ${n} ${NRHS}
                echo ","${m}","${n}","${NRHS}","${threads}","
            done
        done
    done
done

echo "MKL,"
echo "time,m,n,NRHS,threads,"
for m in 4000 12000 20000 30000; do
    for n in 4000 12000 20000 30000; do 
        for NRHS in 1 ; do 
            for threads in 4 12 24 48 72 96 ; do
                export MKL_NUM_THREADS=${threads}
                ./dgels_mkl_t ${m} ${n} ${NRHS}
                echo ","${m}","${n}","${NRHS}","${threads}","
            done
        done
    done
done