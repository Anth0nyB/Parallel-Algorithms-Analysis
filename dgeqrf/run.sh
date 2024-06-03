#!/bin/bash 
# 
#SBATCH --job-name=sockets_close_dgeqrf
#SBATCH --output=sockets_close_console.out
# 
#SBATCH --partition=cpu
#SBATCH --nodes=1 
#SBATCH --ntasks=1 
#SBATCH --cpus-per-task=48
#SBATCH --exclusive
#SBATCH --mem-per-cpu=4G
#SBATCH --time=4-00:00:00 
#
#SBATCH --mail-user=abryson@scu.edu
#SBATCH --mail-type=END


# Load the modules and environment variables
source ../environment.env

# Define variables
events="../events/hand_picked_events.txt"
job="sockets_close_dgeqrf"

# Print column labels for OpenBLAS
printf "problem size," > "data/${job}_ob.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/${job}_ob.csv"
done < "$events"
printf "Runtime,\n" >> "data/${job}_ob.csv"

# Run OpenBLAS
for m in 20000 50000; do
    for n in 1000 4000; do 
        for threads in 1 4 12 20 36 48; do
            export OMP_NUM_THREADS=${threads}
            ./dgeqrf_ob ${m} ${n} >> "data/${job}_ob.csv"
        done
    done
done



# Print column labels for MKL
printf "problem size," > "data/${job}_mkl.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/${job}_mkl.csv"
done < "$events"
printf "Runtime,\n" >> "data/${job}_mkl.csv"

# Run MKL
for m in 20000 50000; do
    for n in 1000 4000; do 
        for threads in 1 4 12 20 36 48; do
            export MKL_NUM_THREADS=${threads}
            export OMP_NUM_THREADS=${threads}   # for omp parallel blocks that set up counters
            ./dgeqrf_mkl ${m} ${n} >> "data/${job}_mkl.csv"
        done
    done
done

