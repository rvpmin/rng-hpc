#!/bin/bash

N=${1:-300} 
PROCS=(2 3 4 5 6 7 8)
OUTPUT="tiempos.txt"

echo "procs real user sys" > $OUTPUT

for np in "${PROCS[@]}"; do
    echo "Ejecutando con $np procesos..."

    #result=$(mpiexec -n $np -f /data/user1/mymachinefile ./rng $N)
    result=$(mpiexec -n $np -f /data/user1/mymachinefile -env FI_PROVIDER tcp ./rng_mpi $N)
    
    real=$(echo "$result" | grep "^real" | awk '{print $2}')
    user=$(echo "$result" | grep "^user" | awk '{print $2}')
    sys=$(echo "$result" | grep "^sys"  | awk '{print $2}')
    
    echo "$np $real $user $sys" >> $OUTPUT
done

echo "Tiempos guardados en $OUTPUT"