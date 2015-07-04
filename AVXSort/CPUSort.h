#include "stdafx.h"

const int rUnit = 8;
const int bufferSize = 128;

extern "C"
{
	void AVXBitonicSort(int *input, int *output, int length);
	void OddCopy(int *input, int *output, int length);
	void AVXMergeSort(int *input, int *output, int len1, int len2);
	void AVXMergeSortEnd(int *input, int *output, int len1, int len2);
	void CoreSortStage1(int *input, int * const output, size_t len);
}

void AVXSort(int *begin, int *start);
void ompAVXSort(int *begin, int *end);

