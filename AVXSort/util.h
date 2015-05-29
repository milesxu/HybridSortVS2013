#include "stdafx.h"
#pragma once


class DataHelper
{
private:
	int *data;
	size_t length;
	std::mt19937 engine;
	std::uniform_int_distribution<> dist;
public:
	DataHelper(unsigned long seed, size_t N, size_t low, size_t high,
			   bool check = 0);
	~DataHelper();
	void generateData(int *arr, bool nonRepeat = 1);
	bool checkResult(int *arr);
	template <typename T> void outputData(T *arr);
};

