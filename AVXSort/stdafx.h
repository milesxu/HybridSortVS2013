// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#define THREADS 8
typedef std::vector<int, boost::alignment::aligned_allocator<int, 32>> 
aligened_vector;

