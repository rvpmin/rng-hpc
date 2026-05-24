#!/bin/bash

N=1000
PROCS=(1 2 4 8 16)
OUTPUT="tiempos.txt"

echo "procs real user sys" > $OUTPUT

for np in "${PROCS[@]}"; do
    echo "Ejecutando con $np procesos..."
    
    result=$(mpirun -np $np ./rng_mpi $N)
    
    real=$(echo "$result" | grep "^real" | awk '{print $2}')
    user=$(echo "$result" | grep "^user" | awk '{print $2}')
    sys=$(echo "$result" | grep "^sys"  | awk '{print $2}')
    
    echo "$np $real $user $sys" >> $OUTPUT
done

echo "Tiempos guardados en $OUTPUT"