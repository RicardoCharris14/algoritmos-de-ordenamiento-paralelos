#include <iostream>
#include "parallel_merge.hpp"
#include <random>
#include <algorithm>

int main(){

    std::vector<int> v1;
    std::vector<int> v2;
    std::vector<int> merge;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    for (int i=0 ; i<1000 ; i++){
        v1.push_back(dist(gen));
    }
    std::sort(v1.begin(), v1.end());

    for (int i=0 ; i<1000 ; i++){
        v2.push_back(dist(gen));
    }
    std::sort(v2.begin(), v2.end());

    int n = v1.size() + v2.size();
    parallel_merge(v1, v2, merge, 0, v1.size()-1, 0, v2.size()-1, n, 1.1);

    std::cout << n << " | " << merge.size() << std::endl;
    for (int i=0 ; i<merge.size() ; i++){
        std::cout << merge[i] << " ";
    }

    return 0;
}