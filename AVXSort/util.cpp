#include "stdafx.h"
#include "util.h"


DataHelper::DataHelper(unsigned long seed, size_t N, int low, int high,
					   bool check): length(N), engine(seed), dist(low, high)
{
	if (check)
		data = new int[N];
	else
		data = NULL;
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
		std::for_each(arr, arr + length, [&](int &i){ i = dist(engine); });
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
				std::cout << "sort result check failed!" << std::endl;
				return false;
			}
		}
	}
	else
		for (size_t i = 0; i < length - 1; i++) {
			if (arr[i] > arr[i + 1])
			{
				std::cout << "sort result check failed!" << std::endl;
				return false;
			}
		}
	std::cout << "sort success!" << std::endl;
	return true;

}
