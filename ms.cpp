#include <iostream>
#include <omp.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include "merge.h"

// Umbral de corte para la recursión en paralelo
const int UMBRAL = 1000;

void sequentialMergeSort(std::vector<int>& arr, int left, int right) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    
    sequentialMergeSort(arr, left, mid);
    sequentialMergeSort(arr, mid + 1, right);
    
    merge(arr, left, mid, right);
}

void parallelMergeSort(std::vector<int>& arr, int left, int right) {

    if (right - left < UMBRAL) {
        sequentialMergeSort(arr, left, right);
        return;
    }

    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    
    #pragma omp task shared(arr)
    parallelMergeSort(arr, left, mid);
    
    #pragma omp task shared(arr)
    parallelMergeSort(arr, mid + 1, right);
    
    #pragma omp taskwait
    
    merge(arr, left, mid, right);
}