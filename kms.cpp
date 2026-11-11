#include <iostream>
#include <omp.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <queue>

const int UMBRAL = 1000;

struct HeapNode {
    int val;
    int subArrayIdx;
    int elementIdx;

    bool operator>(const HeapNode& other) const {
        return val > other.val;
    }
};

void kWayMerge(std::vector<int>& A, int l, int r, const std::vector<int>& points) {
    int n = r - l + 1;
    std::vector<int> temp(n);
    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> minHeap;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        if (points[i] < points[i+1]) {
            minHeap.push({A[points[i]], (int)i, points[i]});
        }
    }

    for (int i = 0; i < n; ++i) {
        if (minHeap.empty()) break;

        HeapNode root = minHeap.top();
        minHeap.pop();

        temp[i] = root.val;

        int nextElementIdx = root.elementIdx + 1; 
        if (nextElementIdx < points[root.subArrayIdx + 1]) {
            minHeap.push({A[nextElementIdx], root.subArrayIdx, nextElementIdx});
        }
    }

    for (int i = 0; i < n; ++i) {
        A[l + i] = temp[i];
    }
}

void sequentialkWayMergeSort(std::vector<int>& arr, int l, int r, int k) {
    if (l >= r) return;

    std::vector<int> div_points;
    div_points.push_back(l);
    for (int i = 1; i < k; ++i) {
        div_points.push_back(l + i * (r - l + 1) / k);
    }
    div_points.push_back(r + 1);

    for (int i = 0; i < k; ++i) {
        sequentialkWayMergeSort(arr, div_points[i], div_points[i+1] - 1, k);
    }

    kWayMerge(arr, l, r, div_points);
}

void _parallelkWayMergeSort(std::vector<int>& arr, int l, int r, int k) {
    if (r - l < UMBRAL) {
        sequentialkWayMergeSort(arr, l, r, k);
        return;
    }

    if (l >= r) return;

    std::vector<int> div_points;
    div_points.push_back(l);
    for (int i = 1; i < k; ++i) {
        div_points.push_back(l + i * (r - l + 1) / k);
    }
    div_points.push_back(r + 1);

 

    for (int i = 0; i < k; ++i) {
        #pragma omp task shared(arr) firstprivate(i, div_points)
        _parallelkWayMergeSort(arr, div_points[i], div_points[i+1] - 1, k);
    }
    #pragma omp taskwait

    kWayMerge(arr, l, r, div_points);
}

void parallelkWayMergeSort(std::vector<int>& arr, int k) {
    #pragma omp parallel
    {
        #pragma omp single
        _parallelkWayMergeSort(arr, 0, arr.size() - 1, k);
    }
}