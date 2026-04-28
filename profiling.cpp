#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include "complete_p_merge_sort.cpp" 

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0] << " <n_potencia> <k> <hilos>" << std::endl;
        return 1;
    }

    int n_pow = std::atoi(argv[1]);
    long long n = 1LL << n_pow; 
    int k = std::atoi(argv[2]);
    int hilos = std::atoi(argv[3]);

    // omp_set_num_threads(hilos);
    std::vector<int> data(n);
    
    std::srand(42); 
    std::generate(data.begin(), data.end(), std::rand);

    parallelCompleteMergeSort(data, k);

    if (!std::is_sorted(data.begin(), data.end())) {
        std::cerr << "Error: El arreglo no quedó ordenado." << std::endl;
        return 1;
    }

    return 0;
}