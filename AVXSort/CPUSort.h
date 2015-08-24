#include "stdafx.h"

typedef std::vector<int>::iterator ivec_iter;
typedef std::vector<std::pair<int*, int*>> boundary;
typedef std::vector<int, 
    boost::alignment::aligned_allocator<int, 32>> align_vector;

const int rUnit = 8;
const int bufferSize = 128;

extern "C"
{
	void AVXBitonicSort(int *input, int *output, int length);
	void OddCopy(int *input, int *output, int length);
	void AVXMergeSort(int *input, int *output, int len1, int len2);
	void AVXMergeSortEnd(int *input, int *output, int len1, int len2);
	void CoreSortStage1(int *input, int * const output, size_t len);
	void MergeUseAVX(int *start1, int *end1, int *stat2, int *end2, 
        int *output, bool first = false, bool last = false);
    int *CopyUseAVX(int *ibegin, int *iend, int *obegin);
}

void AVXSort(int *begin, int *start);
void ompAVXSort(int *begin, int *end);
void recursiveMultiwayMerge(int *begin, int *end);
