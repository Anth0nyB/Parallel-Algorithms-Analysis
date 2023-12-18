#!/bin/bash 
# 
#SBATCH --job-name=dgemm_t
#SBATCH --output=dgemm_t.csv
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
make dgemm > /dev/null

# Run the Program
echo "OpenBLAS,"
echo "time,m,n,k,threads,"
for m in 10000 30000 50000; do
    for k in 10000 30000 50000; do 
        for n in 10000 30000 50000; do 
            for threads in 4 12 24 48 72 96 ; do
                export OMP_NUM_THREADS=${threads}
                ./dgemm_ob_t ${m} ${k} ${n}
                echo ","${m}","${k}","${n}","${threads}","
            done
        done
    done
done

echo "MKL,"
echo "time,m,n,k,threads,"
for m in 10000 30000 50000; do
    for k in 10000 30000 50000; do 
        for n in 10000 30000 50000; do 
            for threads in 4 12 24 48 72 96 ; do
                export MKL_NUM_THREADS=${threads}
                ./dgemm_mkl_t ${m} ${k} ${n}
                echo ","${m}","${k}","${n}","${threads}","
            done
        done
    done
done
