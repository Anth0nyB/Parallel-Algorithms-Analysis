#!/bin/bash 
# 
#SBATCH --job-name=dgeqrf
#SBATCH --output=console.out
# 
#SBATCH --partition=cpu
#SBATCH --nodes=1 
#SBATCH --ntasks=1 
#SBATCH --cpus-per-task=96
#SBATCH --mem-per-cpu=4G
#SBATCH --time=4-00:00:00 
#
#SBATCH --mail-user=abryson@scu.edu
#SBATCH --mail-type=END


# Load the modules and environment variables
source ../environment.env

# Compile the program
make > /dev/null

# Run the Program
events="../events/hand_picked_events.txt"

printf "problem size," > "data/dgeqrf_ob.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/dgeqrf_ob.csv"
done < "$events"
printf "Runtime,\n" >> "data/dgeqrf_ob.csv"

for m in 30000 50000 70000; do
    for n in 1000 4000; do 
        for threads in 1 4 12 20 36 48; do
            export OMP_NUM_THREADS=${threads}
            ./dgeqrf_ob ${m} ${n} >> "data/dgeqrf_ob.csv"
        done
    done
done

printf "problem size," > "data/dgeqrf_mkl.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/dgeqrf_mkl.csv"
done < "$events"
printf "Runtime,\n" >> "data/dgeqrf_mkl.csv"

for m in 30000 50000 70000; do
    for n in 1000 4000; do 
        for threads in 1 4 12 20 36 48; do
            export MKL_NUM_THREADS=${threads}
            export OMP_NUM_THREADS=${threads}   # for omp parallel blocks that set up counters
            ./dgeqrf_mkl ${m} ${n} >> "data/dgeqrf_mkl.csv"
        done
    done
done

make clean > /dev/null

