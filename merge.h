#ifndef MERGE_H
#define MERGE_H

#include <vector>

/**
 * Función merge para combinar dos subarrays ordenados
 * @param arr Vector a ordenar
 * @param left Índice inicio del subarray izquierdo
 * @param mid Índice final del subarray izquierdo
 * @param right Índice final del subarray derecho
 */
void merge(std::vector<int>& arr, int left, int mid, int right) {
    std::vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;
    
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    
    for (int p = 0; p < k; p++) arr[left + p] = temp[p];
}

#endif
