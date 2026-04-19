#ifndef PARALLEL_MERGE_HPP
#define PARALLEL_MERGE_HPP

#include <vector>
#include <iostream>
#include <cmath>
#include "omp.h"

int rank(const std::vector<int>& v, int li, int ri, int num){

    int l = li, r = ri;
    int res = li - 1;


    while (l <= r){

        int mid = l + (r - l)/2;

        if (v[mid] <= num){
            l = mid + 1;
            res = mid;
        } else {
            r = mid - 1;
        }
    }
    
    return res;
}

std::vector<int> partition(const std::vector<int>& v1, const std::vector<int>& v2, int v1li, int v1ri, int v2li, int v2ri, int n){

    double step = std::log2(n);
    if (step < 1.0) step = 1.0;
    int partition_amount = std::ceil((v1ri - v1li + 1)/step);

    std::vector<int> partition_v2(partition_amount+1);
    partition_v2[0] = v2li - 1;

    #pragma omp parallel for
    for (int i=1 ; i<=partition_amount ; i++){
        int idx = v1li + i*step;
        if (idx > v1ri) idx = v1ri;
        idx = rank(v2, v2li, v2ri, v1[idx]);
        partition_v2[i] = idx;
    }
    
    return partition_v2;
}

void sequential_merge(const std::vector<int>& v1, const std::vector<int>& v2, std::vector<int>& merged_v, int v1li, int v1ri, int v2li, int v2ri){

    int n = (v1ri - v1li + 1) + (v2ri - v2li + 1), i=0, j=0;
    merged_v.resize(n);
    
    for(int k=0 ; k<n ; k++){
        if (v1li + i > v1ri) merged_v[k] = v2[v2li + j++];
        else if (v2li + j > v2ri) merged_v[k] = v1[v1li + i++];
        else if (v1[v1li + i] <= v2[v2li + j]) merged_v[k] = v1[v1li + i++];
        else merged_v[k] = v2[v2li + j++];
    }
}

void parallel_merge(const std::vector<int>& v1, const std::vector<int>& v2, std::vector<int>& merged_arrays, int v1li, int v1ri, int v2li, int v2ri, int n, float c){

    if (v1li > v1ri) {
        if (v2li <= v2ri){
            merged_arrays.insert(merged_arrays.end(), v2.begin() + v2li, v2.begin() + v2ri + 1);
        }
        return;
    }
    if (v2li > v2ri) {
        if (v1li <= v1ri) merged_arrays.insert(merged_arrays.end(), v1.begin() + v1li, v1.begin() + v1ri + 1);
        return;
    }

    int size = (v1ri - v1li + 1) + (v2ri - v2li + 1);
    double step = std::log2(size);
    if (step < 1.0) step = 1.0;
    int partition_amount = std::ceil((v1ri - v1li + 1)/step);

    std::vector<int> partition_v2 = partition(v1, v2, v1li, v1ri, v2li, v2ri, size);
    int actual_partitions = partition_v2.size() - 1;
    std::vector<std::vector<int>> sub_arrays(actual_partitions);


    int reserved_space = (v1ri - v1li + 1) + (v2ri - v2li + 1);
    merged_arrays.reserve(reserved_space);

    #pragma omp parallel for
    for (int i=1 ; i<=actual_partitions ; i++){

        std::vector<int> partition_merge;

        int v1_p_lidx = (i == 1) ? v1li : (v1li + (i-1)*step + 1);
        int v1_p_ridx = v1li + i*step;
        if (v1_p_ridx > v1ri) v1_p_ridx = v1ri;
        int v2_p_lidx = partition_v2[i-1] + 1;
        int v2_p_ridx = partition_v2[i];
        if (v2_p_ridx > v2ri) v2_p_ridx = v2ri;

        if (v1_p_lidx > v1_p_ridx && v2_p_lidx > v2_p_ridx) continue;

        if((v2_p_ridx - v2_p_lidx + 1) <= c*std::log2(n)){
            sequential_merge(v1, v2, partition_merge, v1_p_lidx, v1_p_ridx, v2_p_lidx, v2_p_ridx);
        } else {
            parallel_merge(v2, v1, partition_merge, v2_p_lidx, v2_p_ridx, v1_p_lidx, v1_p_ridx, n, c);
        }
        sub_arrays[i-1] = std::move(partition_merge);
    }

    for (int i=0 ; i<actual_partitions ; i++){
        merged_arrays.insert(merged_arrays.end(), sub_arrays[i].begin(), sub_arrays[i].end());
    }
    if (partition_v2.back() < v2ri){
        merged_arrays.insert(merged_arrays.end(), v2.begin() + partition_v2.back()+1, v2.begin() + v2ri + 1);
    }

}

#endif
