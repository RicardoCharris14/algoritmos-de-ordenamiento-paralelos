#!/bin/bash

METRICS="cycles,instructions,cache-references,cache-misses"

REPS=1  
LOWER=20
UPPER=26
STEP=2
HILOS=(2 4 6 8 10) 

sudo sysctl -w kernel.perf_event_paranoid=-1


for k in 4 8 16; do
    perf stat -e $METRICS -o "profile_kway_${k}_sec.txt" ./expkway results_temp.csv $REPS $LOWER $UPPER $STEP sec $k

    for p in "${HILOS[@]}"; do
        perf stat -e $METRICS -o "profile_kway_${k}_par_${p}threads.txt" ./expkway results_temp.csv $REPS $LOWER $UPPER $STEP par $k $p
    done
done

perf stat -e $METRICS -o "profile_normal_sec.txt" ./exp results_temp.csv $REPS $LOWER $UPPER $STEP sec

for p in "${HILOS[@]}"; do
    perf stat -e $METRICS -o "profile_normal_par_${p}threads.txt" ./exp results_temp.csv $REPS $LOWER $UPPER $STEP par $p
done
