// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define NOMINMAX

#include "targetver.h"

#include <malloc.h>
//#include <stdio.h>
#include <tchar.h>
#include <omp.h>
#include <fstream>
#include <iostream>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <new>
#include <iterator>
#include <immintrin.h>
#include <xmmintrin.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <functional>
#include <numeric>
#include <ratio>
#include <cstdint>
#include <map>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/align/aligned_allocator.hpp>
#include <boost/sort/spreadsort/spreadsort.hpp>

#include <windows.h>



// TODO:  在此处引用程序需要的其他头文件
#define THREADS 8
typedef std::vector<int, boost::alignment::aligned_allocator<int, 32>> 
aligened_vector;

