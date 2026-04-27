#ifndef PARALLEL_MERGE_HPP
#define PARALLEL_MERGE_HPP

#include <vector>
#include <algorithm>
#include <cmath>
#include "omp.h"

static inline int rank(const std::vector<int>& v, int li, int ri, int num) {
    int l = li, r = ri, res = li - 1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (v[mid] <= num) { l = mid + 1; res = mid; }
        else               { r = mid - 1; }
    }
    return res;
}

static std::vector<int> partition(
    const std::vector<int>& v1, const std::vector<int>& v2,
    int v1li, int v1ri, int v2li, int v2ri, int step)
{
    int v1_size = v1ri - v1li + 1;
    int t = (int)std::ceil((double)v1_size / step);
    if (t < 1) t = 1;

    std::vector<int> pv2(t + 1);
    pv2[0] = v2li - 1;

    for (int i = 1; i <= t; i++) {
        int idx = v1li + i * step;
        if (idx > v1ri) idx = v1ri;
        pv2[i] = rank(v2, v2li, v2ri, v1[idx]);
    }

    return pv2;
}

static void sequential_merge(
    const std::vector<int>& v1, const std::vector<int>& v2,
    std::vector<int>& out, int v1li, int v1ri, int v2li, int v2ri)
{
    int a = (v1ri >= v1li) ? (v1ri - v1li + 1) : 0;
    int b = (v2ri >= v2li) ? (v2ri - v2li + 1) : 0;
    out.resize(a + b);
    int i = 0, j = 0, k = 0;
    while (i < a && j < b)
        out[k++] = (v1[v1li+i] <= v2[v2li+j]) ? v1[v1li+i++] : v2[v2li+j++];
    while (i < a) out[k++] = v1[v1li + i++];
    while (j < b) out[k++] = v2[v2li + j++];
}

static void _parallel_merge(
    const std::vector<int>& v1, const std::vector<int>& v2,
    std::vector<int>& merged,
    int v1li, int v1ri, int v2li, int v2ri,
    int n, float c)
{
    if (v1li > v1ri) {
        if (v2li <= v2ri)
            merged.insert(merged.end(), v2.begin()+v2li, v2.begin()+v2ri+1);
        return;
    }
    if (v2li > v2ri) {
        merged.insert(merged.end(), v1.begin()+v1li, v1.begin()+v1ri+1);
        return;
    }

    int step = (int)std::log2((double)n);
    if (step < 1) step = 1;
    int threshold = (int)(c * std::log2((double)n));
    if (threshold < 1) threshold = 1;

    std::vector<int> pv2 = partition(v1, v2, v1li, v1ri, v2li, v2ri, step);
    int t = (int)pv2.size() - 1;

    std::vector<std::vector<int>> sub(t);

    merged.reserve((v1ri-v1li+1) + (v2ri-v2li+1));

    #pragma omp taskloop shared(v1, v2, sub, pv2) firstprivate(step, threshold, v1li, v1ri, v2li, v2ri, n, c)
    for (int i = 1; i <= t; i++) {
        int a_li = (i == 1) ? v1li : (v1li + (i-1)*step + 1);
        int a_ri = std::min(v1li + i*step, v1ri);
        int b_li = pv2[i-1] + 1;
        int b_ri = std::min(pv2[i], v2ri);

        if (!(a_li > a_ri && b_li > b_ri)) {
            std::vector<int> local;
            int b_size = (b_ri >= b_li) ? (b_ri - b_li + 1) : 0;

            if (b_size <= threshold) {
                sequential_merge(v1, v2, local, a_li, a_ri, b_li, b_ri);
            } else {
                _parallel_merge(v2, v1, local, b_li, b_ri, a_li, a_ri, n, c);
            }
            sub[i-1] = std::move(local);
        }
    }

    for (int i = 0; i < t; i++)
        merged.insert(merged.end(), sub[i].begin(), sub[i].end());

    if (pv2.back() < v2ri)
        merged.insert(merged.end(), v2.begin()+pv2.back()+1, v2.begin()+v2ri+1);
}

void parallel_merge(
    const std::vector<int>& v1, const std::vector<int>& v2,
    std::vector<int>& merged,
    int v1li, int v1ri, int v2li, int v2ri,
    int n, float c, int UMBRAL = 1000)
{
    #pragma omp parallel
    {
        #pragma omp single
        _parallel_merge(v1, v2, merged, v1li, v1ri, v2li, v2ri, n, c);
    }
}

#endif