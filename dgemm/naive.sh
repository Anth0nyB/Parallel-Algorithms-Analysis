#!/bin/bash 
# 
#SBATCH --job-name=dgemm_naive
#SBATCH --output=n_console.out
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
make naive > /dev/null

# Run the Program
events="../events/hand_picked_events.txt"

printf "problem size," > "data/dgemm_naive.csv"
while IFS= read -r line; do
    printf "%s," "$line" >> "data/dgemm_naive.csv"
done < "$events"
printf "Runtime,\n" >> "data/dgemm_naive.csv"

for m in 2000 4000 8000; do
    for k in 4000; do 
        # for n in  30000; do
            for threads in 1 4 12 20 36 48; do
                export OMP_NUM_THREADS=${threads}
                ./dgemm_naive ${m} ${k} ${m} >> "data/dgemm_naive.csv"
            done
        # done
    done
done

make clean > /dev/null
