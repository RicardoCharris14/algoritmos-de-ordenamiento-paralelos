#include <queue>
#include "parallel_merge.hpp"

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

void completeMerge(std::vector<int>& A, int l, int r, const std::vector<int>& points, float c) {
    std::vector<std::vector<int>> subArrays;
    int nSubArrays = 0;
    for (int i = 0; i < points.size() - 1; ++i) {
        if (points[i] < points[i+1]) {
            nSubArrays++;
            std::vector<int> temp;
            
            temp.insert(temp.end(), A.begin() + points[i], A.begin() + points[i+1]);
            subArrays.push_back(temp);
        }
    }

    if (subArrays.empty()) return;

    int iterations = std::ceil(std::log2(nSubArrays));
    int li = 0;
    
    for (int i=0 ; i<iterations ; i++){
        int size = subArrays.size();
        int num_merges = (size - li) / 2;
        int remainder = (size - li) % 2;
        
        subArrays.resize(size + num_merges + remainder); 

        #pragma omp taskloop shared(subArrays) firstprivate(li, size)
        for (int j = 0; j < num_merges; ++j) {
            int idx = li + j * 2;
            std::vector<int> merge;
            
            _parallel_merge(subArrays[idx], subArrays[idx+1], merge, 0, subArrays[idx].size()-1, 0, subArrays[idx+1].size()-1, subArrays[idx].size() + subArrays[idx+1].size(), c);
            subArrays[size + j] = std::move(merge);
        }
        
        if (remainder == 1) {
            subArrays[size + num_merges] = std::move(subArrays[size - 1]);
        }
        li = size;
    }

    std::copy(subArrays.back().begin(), subArrays.back().end(), A.begin() + l);

}

void sequentialCompleteMergeSort(std::vector<int>& arr, int l, int r, int k) {
    if (l >= r) return;

    std::vector<int> div_points;
    div_points.push_back(l);
    for (int i = 1; i < k; ++i) {
        div_points.push_back(l + i * (r - l + 1) / k);
    }
    div_points.push_back(r + 1);

    for (int i = 0; i < k; ++i) {
        sequentialCompleteMergeSort(arr, div_points[i], div_points[i+1] - 1, k);
    }

    kWayMerge(arr, l, r, div_points);
}

void _parallelCompleteMergeSort(std::vector<int>& arr, int l, int r, int k, float c) {
    if (r - l < UMBRAL) {
        sequentialCompleteMergeSort(arr, l, r, k);
        return;
    }

    if (l >= r) return;

    std::vector<int> div_points;
    div_points.push_back(l);
    for (int i = 1; i < k; ++i) {
        div_points.push_back(l + i * (r - l + 1) / k);
    }
    div_points.push_back(r + 1);

 
    #pragma omp taskloop shared(arr, div_points) firstprivate(k)
    for (int i = 0; i < k; ++i) {
        _parallelCompleteMergeSort(arr, div_points[i], div_points[i+1] - 1, k, c);
    }

    completeMerge(arr, l, r, div_points, c);
}

void parallelCompleteMergeSort(std::vector<int>& arr, int k, float c = 2.0f) {
    #pragma omp parallel
    {
        #pragma omp single
        _parallelCompleteMergeSort(arr, 0, arr.size() - 1, k, c);
    }
}