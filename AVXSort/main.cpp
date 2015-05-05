#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
    const int N = rUnit * rUnit * 2;
    const int arr = rUnit;
    int *input = (int *)_mm_malloc(N * sizeof(int), 32);
    // int *output = (int *)_mm_malloc(N * sizeof(int), 32);
    std::mt19937 engine(2014120421);
    std::uniform_int_distribution<> dist(0, 1000);
    std::for_each(input, input + N, [&](int &i){ i = dist(engine); });
    // __m256i data[arr];
    // int *temp = input;
    // loadData<arr>(temp, data);

    auto num = 0;
    std::for_each(input, input + N, [&](int i){
        std::cout << std::setw(3) << i << " ";
        num += 1;
        if (num % 8 == 0)
            std::cout << std::endl;
    });
    std::cout << std::endl;
    // temp = output;
    // bitonicSort<arr>(data, temp);
    //storeData<arr>(temp, data);
    //AVXSort(input, N);
    //std::cout << "sort complete." << std::endl;
    //AVXBitonicSort(input, input, N >> 6);
    AVXBitonicSort(input, input, 2);
    num = 0;
    std::for_each(input, input + N, [&](int i){
        std::cout << std::setw(3) << i << " ";
        num += 1;
        if (num % 8 == 0)
            std::cout << std::endl;
    });
    std::cout << std::endl;
    int *output = (int *)_mm_malloc(N * sizeof(int), 32);
    // AVXMergeSort(input, output, 64, 64, 0, 0);
    AVXMergeSort(input, output, 64 << 2, 64 << 2);
    //OddCopy(input, output, N);
    std::for_each(output, output + N, [&](int i){
        std::cout << std::setw(3) << i << " ";
        num += 1;
        if (num % 8 == 0)
            std::cout << std::endl;
    });
    _mm_free(input);
    _mm_free(output);
    return 0;
}
