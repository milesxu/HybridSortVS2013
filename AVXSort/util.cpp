#include "stdafx.h"
#include "util.h"
#include "CPUSort.h"


DataHelper::DataHelper(unsigned long seed, size_t N, size_t low, size_t high,
                       bool check): length(N), engine(seed),
                                    dist((int)low, (int)high)
{
    if (check)
        data = new int[N];
    else
        data = nullptr;
}

DataHelper::~DataHelper()
{
    if (data)
        delete [] data;
}

void DataHelper::generateData(int *arr, bool nonRepeat)
{
    if (nonRepeat)
    {
        std::unordered_set<int> tempset;
        while (tempset.size() < length)
            tempset.insert(dist(engine));		
        std::copy(tempset.begin(), tempset.end(),
                  stdext::make_checked_array_iterator(arr, length, 0));
    }
    else
        std::generate(arr, arr + length, [&]{ return dist(engine); });
    if (data)
        std::copy(stdext::make_checked_array_iterator(arr, length, 0),
                  stdext::make_checked_array_iterator(arr, length, length),
                  stdext::make_checked_array_iterator(data, length, 0));
}

bool DataHelper::checkResult(int *arr)
{
    if (data)
    {
        std::sort(data, data + length);
        for (size_t i = 0; i < length; i++) {
            if (data[i] != arr[i])
            {
                std::cout << "checked sort result check failed at " 
                    << i << "!" << std::endl;
                return false;
            }
        }
    }
    else
        for (size_t i = 0; i < length - 1; i++) {
            if (arr[i] > arr[i + 1])
            {
                std::cout << "unchecked sort result check failed at "
                    << i << "!" << std::endl;
                return false;
            }
        }
    std::cout << "sort success!" << std::endl;
    return true;

}

template <typename T> void DataHelper::outputData(T *arr)
{
    size_t j = 0;
    std::for_each(stdext::make_checked_array_iterator(arr, length, 0),
                  stdext::make_checked_array_iterator(arr, length, length),
                  [&](T i){
                      std::cout << std::setw(3) << i << " ";
                      j++;
                      if (j % 8 == 0)
                          std::cout << std::endl;
                  });
}

template void DataHelper::outputData<int>(int *arr);

size_t getUnitLengthPerCore()
{
    DWORD buff_size = 0;
    size_t cacheByte = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buff = nullptr;
    GetLogicalProcessorInformation(nullptr, &buff_size);
    buff = static_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION *>
        (malloc(buff_size));
    GetLogicalProcessorInformation(&buff[0], &buff_size);
    for (size_t i = 0;
         i < buff_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); i++) {
        if (buff[i].Relationship == RelationCache && buff[i].Cache.Level == 1)
        {
            cacheByte = buff[i].Cache.Size;
			break;
        }
    }
    free(buff);
    return cacheByte;
}

void resultTiming(int testTime, int *data, size_t length,
                  std::function<void (int *, int *)> sortF)
{
	std::vector<int> temp(data, data + length);
	typedef std::chrono::milliseconds ms;
	//typedef std::chrono::duration<double, std::micro> ms;
	typedef std::chrono::steady_clock clock;
	std::vector<ms> results;
	//sortF(data, length);
    auto start = clock::now();
    sortF(data, data + length);
    auto end = clock::now();
    std::cout << (std::chrono::duration_cast<ms>(end - start)).count() << std::endl;
	/*std::transform(boost::counting_iterator<int>(0),
				   boost::counting_iterator<int>(testTime << 1),
				   std::inserter(results, results.begin()),
				   [&](int i){
					   std::copy(temp.begin(), temp.end(), data);
					   auto start = clock::now();
					   sortF(data, length);
					   auto end = clock::now();
					   //std::cout << std::chrono::duration_cast<ms>(end-start).count() << std::endl;
					   return std::chrono::duration_cast<ms>(end - start);
					   });*/
	for (auto i = 0; i < (testTime << 1); i++) {
		std::copy(temp.begin(), temp.end(), data);
		start = clock::now();
		sortF(data, data + length);
		end = clock::now();
        std::cout << std::chrono::duration_cast<ms>(end - start).count() << std::endl;
		results.push_back(std::chrono::duration_cast<ms>(end - start));
		}
    std::cout << "transform complete;" << std::endl;
	std::nth_element(results.begin(), results.begin() + testTime, results.end());
	ms sum = std::accumulate(results.begin(), results.begin() + testTime, ms(0));
	sum /= testTime;
    std::cout << "function running time(in microseconds): ";
    std::cout << sum.count();
    std::cout << std::endl;
}

int_fast64_t resultTimingWin(int testTime, int *data, size_t length,
				  std::function<void (int *, int *)> sortF)
{
	std::vector<int> temp(data, data + length);
	LARGE_INTEGER start, end, elapsed, frequency;
	QueryPerformanceFrequency(&frequency);
	std::vector<int_fast64_t> results;
	sortF(data, data + length);
	std::transform(boost::counting_iterator<int>(0),
				   boost::counting_iterator<int>(testTime << 1),
				   std::inserter(results, results.begin()),
				   [&](int i){
					   std::copy(temp.begin(), temp.end(), data);
					   QueryPerformanceCounter(&start);
					   sortF(data, data + length);
					   QueryPerformanceCounter(&end);
					   elapsed.QuadPart = end.QuadPart - start.QuadPart;
					   elapsed.QuadPart *= 1000000;
					   elapsed.QuadPart /= frequency.QuadPart;
					   //std::cout << elapsed.QuadPart << std::endl;
					   return elapsed.QuadPart;
				   });
	std::nth_element(results.begin(), results.begin() + testTime, results.end());
	int_fast64_t sum = std::accumulate(results.begin(),
									   results.begin() + testTime, 0);
	sum /= testTime;
/*	std::cout << "function average running time(in microseconds): " << sum
			  << std::endl;*/
    return sum;
}


void sortTest(int start, int end, int test_time_i)
{
    std::fstream result("D:\\result.txt", std::ios::app);
    for (auto i = start; i <= end; i *= 2)
    {
        auto input = static_cast<int *>(_mm_malloc(i * sizeof(int), 32));
        DataHelper dh(201412042120150820, i, 0, i * 2, true);
        dh.generateData(input, 0);
        result << i << " "
            //<< resultTimingWin(test_time_i, input, i, std::sort<int *>)
            //<< resultTimingWin(test_time_i, input, i, 
            //boost::sort::spreadsort::spreadsort<int *>)
            << resultTimingWin(test_time_i, input, i, AVXSort)
            << std::endl;
        dh.checkResult(input);
        _mm_free(input);
    }
}

void copyTest(int length, int test_time_i)
{
    LARGE_INTEGER start, end, elapsed, frequency;
    QueryPerformanceFrequency(&frequency);
    auto input = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
    DataHelper dh(201412042120150820, length, 0, length * 2, 0);
    dh.generateData(input, 0);
    std::vector<int_fast64_t> results;
    for (auto i = 0; i < test_time_i * 3; i++)
    {
        auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
        QueryPerformanceCounter(&start);
        //std::copy(input, input + length, temp);
        //CopyUseAVX(input, input + length, temp);
        auto unitLen = (length >> 10);
#pragma omp parallel for num_threads(8) schedule(dynamic)
        for (auto j = 0; j < length; j += unitLen)
        {
            auto tin = input + j;
            //std::copy(tin, tin + unitLen, temp + j);
            CopyUseAVX(tin, tin + unitLen, temp + j);
        }
        QueryPerformanceCounter(&end);
        elapsed.QuadPart = end.QuadPart - start.QuadPart;
        elapsed.QuadPart *= 1000000;
        elapsed.QuadPart /= frequency.QuadPart;
        results.push_back(elapsed.QuadPart);
        _mm_free(temp);
    }
    std::nth_element(results.begin(), results.begin() + test_time_i, 
        results.end());
    int_fast64_t sum = std::accumulate(results.begin(),
        results.begin() + test_time_i, 0);
    sum /= test_time_i;
    std::ofstream test("D:\\result.txt", std::ios::app);
    test << length << " " << sum << std::endl;
    //test.flush();
    //test.close();
    _mm_free(input);
}