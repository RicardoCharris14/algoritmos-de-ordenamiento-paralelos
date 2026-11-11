#!/bin/bash

# Configuración de Repeticiones y Potencias
REPS=32
LOWER=20
UPPER=26
STEP=2
HILOS=(2 4 6 8 10)

echo "Experimentacion con merge sort normal secuencial..."
./exp results_normal_sec.csv $REPS $LOWER $UPPER $STEP sec

for hilos in "${HILOS[@]}"; do
    echo "Experimentacion con merge sort normal paralelo con ${hilos} hilos..."
    ./exp results_normal_par_${hilos}threads.csv $REPS $LOWER $UPPER $STEP par $hilos
done

for k in 4 8 16; do
    echo "Experimentacion con K-Way Mergesort (k=$k) secuencial..."
    ./expkway results_kway_${k}_sec.csv $REPS $LOWER $UPPER $STEP sec $k
    
    for hilos in "${HILOS[@]}"; do
        echo "Experimentacion con K-Way Mergesort (k=$k) paralelo con ${hilos} hilos..."
        ./expkway results_kway_${k}_par_${hilos}threads.csv $REPS $LOWER $UPPER $STEP par $k $hilos
    done
done

for k in 2 4 8 16; do
    echo "Experimentacion con complete Mergesort (k=$k) secuencial..."
    ./expcomplete results_complete_${k}_sec.csv $REPS $LOWER $UPPER $STEP sec $k
    
    for hilos in "${HILOS[@]}"; do
        echo "Experimentacion con complete Mergesort (k=$k) paralelo con ${hilos} hilos..."
        ./expcomplete results_complete_${k}_par_${hilos}threads.csv $REPS $LOWER $UPPER $STEP par $k $hilos
    done
done