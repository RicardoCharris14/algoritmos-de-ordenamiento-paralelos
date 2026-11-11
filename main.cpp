#include <iostream>
#include "complete_p_merge_sort.cpp"
//#include "kms.cpp"
#include <random>
#include <algorithm>

int main(){

    // Verificar la configuración de OpenMP en tiempo de ejecución
    std::cout << "--- Configuracion OpenMP ---" << std::endl;
    std::cout << "Anidamiento activado (legacy): " << omp_get_nested() << std::endl;
    std::cout << "----------------------------" << std::endl;

    std::vector<int> v1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    for (int i=0 ; i<1000000 ; i++){
        v1.push_back(dist(gen));
    }

    parallelCompleteMergeSort(v1, 8, 2);

    int n = v1.size();

    std::cout << n << " | " << v1.size() << std::endl;
    for (int i=0 ; i<v1.size() ; i++){
        std::cout << v1[i] << " ";
    }
    return 0;
}
