#!/bin/bash

N=${1:-300} 
PROCS=(2 3 4 5 6 7 8)
OUTPUT="tiempos.txt"

echo "procs real user sys" > $OUTPUT

for np in "${PROCS[@]}"; do
    echo "Ejecutando con $np procesos..."


    for i in 1 2 3 4; do
        ssh hpc$i "killall -9 rng_mpi 2>/dev/null; killall -9 hydra_pmi_proxy 2>/dev/null" 
    done
    sleep 5

    result=$(mpiexec -n $np -f /data/user1/mymachinefile /data/user1/rng-hpc/barber/rng_mpi $N)
    
    real=$(echo "$result" | grep "^real" | awk '{print $2}')
    user=$(echo "$result" | grep "^user" | awk '{print $2}')
    sys=$(echo "$result" | grep "^sys"  | awk '{print $2}')
    
    echo "$np $real $user $sys" >> $OUTPUT
    sleep 10 
done

echo "Tiempos guardados en $OUTPUT"