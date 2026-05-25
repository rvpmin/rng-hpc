#!/bin/bash

N=${1:-300} 
PROCS=(2 3 4 5 6 7 8)
REPETICIONES=${2:-10}
OUTPUT="tiempos.txt"

echo "procs rep real user sys" > $OUTPUT

run_mpi() {
    local np=$1
    local intentos=3
    local result=""

    for i in $(seq 1 $intentos); do
        for h in 1 2 3 4; do
            ssh hpc$h "killall -9 rng_mpi 2>/dev/null; killall -9 hydra_pmi_proxy 2>/dev/null"
        done
        sleep 5

        result=$(mpiexec -n $np -f /data/user1/mymachinefile /data/user1/rng-hpc/barber/rng_mpi $N)

        if echo "$result" | grep -q "^real"; then
            echo "$result"
            return 0
        fi

        echo "  Intento $i fallido, reintentando..." >&2
        sleep 5
    done

    echo "  ERROR: fallaron $intentos intentos con $np procesos" >&2
    return 1
}

for np in "${PROCS[@]}"; do
    echo "===== $np procesos ====="

    for rep in $(seq 1 $REPETICIONES); do
        echo "  Rep $rep/$REPETICIONES..."

        result=$(run_mpi $np)

        if [ $? -eq 0 ]; then
            real=$(echo "$result" | grep "^real" | awk '{print $2}')
            user=$(echo "$result" | grep "^user" | awk '{print $2}')
            sys=$(echo "$result"  | grep "^sys"  | awk '{print $2}')

            echo "$np $rep $real $user $sys" >> $OUTPUT
            echo "    real=$real user=$user sys=$sys"
        else
            echo "$np $rep ERROR ERROR ERROR" >> $OUTPUT
            echo "  Rep $rep falló"
        fi
    done

done

echo ""
echo "Tiempos guardados en $OUTPUT"
cat $OUTPUT