#!/bin/bash 
# 
#SBATCH --job-name=dgemm
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

printf "problem size," > "data/dgemm_ob.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/dgemm_ob.csv"
done < "$events"
printf "Runtime,\n" >> "data/dgemm_ob.csv"

for m in 10000 20000 30000; do
    for k in 10000 20000 30000; do 
        for n in 10000 20000 30000; do
            for threads in 4 12 24 48 96; do
                export OMP_NUM_THREADS=${threads}
                ./dgemm_ob ${m} ${k} ${n} >> "data/dgemm_ob.csv"
            done
        done
    done
done


events="../events/hand_picked_events.txt"

printf "problem size," > "data/dgemm_mkl.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/dgemm_mkl.csv"
done < "$events"
printf "Runtime,\n" >> "data/dgemm_mkl.csv"

for m in 10000 20000 30000; do
    for k in 10000 20000 30000; do 
        for n in 10000 20000 30000; do
            for threads in 4 12 24 48 96; do
                export MKL_NUM_THREADS=${threads}
                export OMP_NUM_THREADS=${threads}
                ./dgemm_mkl ${m} ${k} ${n} >> "data/dgemm_mkl.csv"
            done
        done
    done
done

make clean > /dev/null
