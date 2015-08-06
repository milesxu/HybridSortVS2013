#include "stdafx.h"
#include "util.h"
#include "CPUSort.h"

extern template void DataHelper::outputData<int>(int *arr);

void avxParamTest()
{
    auto a = new int(1), b = new int(2), c = new int(3), d = new int(4);
    auto e = new int(5);
    auto h = true;
    std::cout << e << std::endl;
    //std::cout << *MergeUseAVX(a, b, c, d, e, false, true) << std::endl;
    /*if (MergeUseAVX(a, b, c, d, e))
    {
        std::cout << "success" << std::endl;
    }
    else
    {
        std::cout << "fail" << std::endl;
    }*/
    //std::cout << +MergeUseAVX(a, b, c, d, e, false, true) << std::endl;
}

void avxMergeTest()
{
    auto n = 64;
    std::random_device rd;
    std::mt19937 mt(rd());
    aligened_vector a(n << 1), b, c;
    std::iota(a.begin(), a.end(), 0);
    std::shuffle(a.begin(), a.end(), mt);
    std::copy(a.begin(), a.begin() + n, std::back_inserter(b));
    std::copy(a.begin() + n, a.end(), std::back_inserter(c));
    std::sort(b.begin(), b.end());
    std::sort(c.begin(), c.end());
    MergeUseAVX(b.data(), b.data() + n, c.data(), c.data() + n, a.data(), 
        true, true);
    std::cout << std::is_sorted(a.begin(), a.end()) << std::endl;
    auto i = 0;
    for (auto j : a)
    {
        std::cout << j << std::setw(3) << " ";
        ++i;
        if (i % 8 == 0)
            std::cout << std::endl;
    }
}

void extractTest()
{
    aligened_vector a(16);
    std::iota(a.begin(), a.end(), 0);
    auto ymm = _mm256_load_si256(reinterpret_cast<__m256i *>(a.data()));
    auto xmm = _mm256_extracti128_si256(ymm, 1);
    auto end = _mm_extract_epi32(xmm, 3);
    std::cout << end << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
    const auto N = 1 << 16;
    //const auto arr = rUnit;
    auto input = static_cast<int *>(_mm_malloc(N * sizeof(int), 32));
    DataHelper dh(2014120421, N, 0, N * 2, true);
    dh.generateData(input);
    //std::cout << "sort begin" << std::endl;
    //AVXSort(input, N);
    //AVXBitonicSort(input, input, N >> 6);
	//resultTiming(5, input, N, std::sort<int *>);
	//resultTimingWin(5, input, N, std::sort<int *>);
	//resultTimingWin(5, input, N, AVXSort);
	ompAVXSort(input, input + N);
    dh.checkResult(input);
    //avxParamTest();
    //avxMergeTest();
    std::cout << "sort complete." << std::endl;
    extractTest();
    //dh.outputData(input);
    _mm_free(input);

    return 0;
}
