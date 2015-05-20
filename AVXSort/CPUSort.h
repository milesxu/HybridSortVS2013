#include "stdafx.h"

const int rUnit = 8;
const int bufferSize = 128;

extern "C"
{
	void AVXBitonicSort(int *input, int *output, int length);
	void OddCopy(int *input, int *output, int length);
	void AVXMergeSort(int *input, int *output, int len1, int len2);
	void AVXMergeSortEnd(int *input, int *output, int len1, int len2);
	void CoreSortStage1(int *input, int *output, size_t len);
}

template<int size>
void loadData(int *&input, __m256i (&data)[size])
{
    for (auto & i : data)
    {
        i = _mm256_load_si256((__m256i *)input);
        input += rUnit;
    }
}

template<int size>
void dataOutput(__m256i (&data)[size], __m256i (&temp)[size],
    int *&output, bool left = true)
{
    //__m256i temp[size];
    const int half = size >> 1;
    const int qtr = half >> 1;
    if (left)
        for (int i = 0; i < half; i++)
            temp[i] = _mm256_permute2x128_si256(data[i], data[i + half], 32);
    else
        for (int i = 0; i < half; i++)
            temp[i] = _mm256_permute2x128_si256(data[i], data[i + half], 49);
    for (int i = 1; i < half; i += 2)
        temp[i] = _mm256_shuffle_epi32(temp[i], 177);
    for (int i = half; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(temp[i - half], temp[i - half + 1], 170);
    for (int i = half + 1; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(temp[i - half - 1], temp[i - half], 85);
    for (int i = half + qtr; i < size; i++)
        temp[i] = _mm256_shuffle_epi32(temp[i], 78);
    for (int i = 0; i < qtr; i++)
        temp[i] = _mm256_blend_epi32(
        temp[i + half], temp[i + half + qtr], 204);
    for (int i = qtr; i < half; i++)
        temp[i] = _mm256_blend_epi32(temp[i + qtr], temp[i + half], 51);
    for (int i = 1; i < half; i += 3)
    {
        temp[i] = _mm256_shuffle_epi32(temp[i], 177);
        temp[i + 1] = _mm256_shuffle_epi32(temp[i + 1], 78);
        temp[i + 2] = _mm256_shuffle_epi32(temp[i + 2], 27);
    }

    for (int i = 0; i < half; ++i)
    {
        _mm256_store_si256((__m256i *)output, temp[i]);
        output += rUnit;
    }
}

template<int size>
void horizentalBitonicSort(__m256i (&data)[size])
{
    __m256i temp[size];

    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 1; i < size; i += 2)
        temp[i] = _mm256_max_epi32(data[i - 1], data[i]);

    for (int i = 0; i < size; i += 4)
    {
        data[i] = _mm256_min_epi32(temp[i], temp[i + 3]);
        data[i + 1] = _mm256_min_epi32(temp[i + 1], temp[i + 2]);
    }
    for (int i = 0; i < size; i += 4)
    {
        data[i + 3] = _mm256_max_epi32(temp[i], temp[i + 3]);
        data[i + 2] = _mm256_max_epi32(temp[i + 1], temp[i + 2]);
    }
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 1; i < size; i += 2)
        temp[i] = _mm256_max_epi32(data[i - 1], data[i]);  // 4 items sort complete.

    const int idx = rUnit - 1;
    for (int i = 0; i < 4; ++i)
        data[i] = _mm256_min_epi32(temp[i], temp[idx - i]);
    for (int i = 0; i < 4; ++i)
        data[idx - i] = _mm256_max_epi32(temp[i], temp[idx - i]);
    for (int i = 0; i < size; i += 4)
    {
        temp[i] = _mm256_min_epi32(data[i], data[i + 2]);
        temp[i + 1] = _mm256_min_epi32(data[i + 1], data[i + 3]);
    }
    for (int i = 0; i < size; i += 4)
    {
        temp[i + 2] = _mm256_max_epi32(data[i], data[i + 2]);
        temp[i + 3] = _mm256_max_epi32(data[i + 1], data[i + 3]);
    }
    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_min_epi32(temp[i], temp[i + 1]);
    for (int i = 1; i < size; i += 2)
        data[i] = _mm256_max_epi32(temp[i - 1], temp[i]); // 8 items sort complete.
}

template<int size>
void bitonicSort(__m256i (&data)[size], int *&output)
{
    horizentalBitonicSort<size>(data);

    const int idx = rUnit - 1;

    __m256i temp[size];

    for (int i = 4; i < size; i++)
        data[i] = _mm256_shuffle_epi32(data[i], 177);
    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_min_epi32(data[i], data[idx - i]);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_max_epi32(data[idx - i], data[i]);

    for (int i = 0; i < 4; i++)
        data[i] = _mm256_blend_epi32(temp[i], temp[idx - i], 170);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_blend_epi32(temp[idx - i], temp[i], 85);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_shuffle_epi32(data[i], 177);

    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_min_epi32(data[i], data[i + 4]);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_max_epi32(data[i - 4], data[i]);

    for (int i = 0; i < size; i += 4)
    {
        data[i] = _mm256_min_epi32(temp[i], temp[i + 2]);
        data[i + 1] = _mm256_min_epi32(temp[i + 1], temp[i + 3]);
    }
    for (int i = 2; i < size; i += 4)
    {
        data[i] = _mm256_max_epi32(temp[i - 2], temp[i]);
        data[i + 1] = _mm256_max_epi32(temp[i - 1], temp[i + 1]);
    }

    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 1; i < size; i += 2)
        temp[i] = _mm256_max_epi32(data[i - 1], data[i]); // 16 sorted finish

    for (int i = 4; i < size; i++)
        temp[i] = _mm256_shuffle_epi32(temp[i], 27);
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_min_epi32(temp[i], temp[idx - i]);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_max_epi32(temp[idx - i], temp[i]);

    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_blend_epi32(data[i], data[i + 4], 170);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_blend_epi32(data[i - 4], data[i], 85);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_shuffle_epi32(temp[i], 177);
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_min_epi32(temp[i], temp[i + 4]);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_max_epi32(temp[i - 4], temp[i]);

    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_blend_epi32(data[i], data[idx - i], 204);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_blend_epi32(data[idx - i], data[i], 51);
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_shuffle_epi32(temp[i], 216);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_shuffle_epi32(temp[i], 114);
    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_min_epi32(data[i], data[i + 4]);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_max_epi32(data[i - 4], data[i]);

    for (int i = 0; i < size; i += 4)
    {
        data[i] = _mm256_min_epi32(temp[i], temp[i + 2]);
        data[i + 1] = _mm256_min_epi32(temp[i + 1], temp[i + 3]);
    }
    for (int i = 2; i < size; i += 4)
    {
        data[i] = _mm256_max_epi32(temp[i - 2], temp[i]);
        data[i + 1] = _mm256_max_epi32(temp[i - 1], temp[i + 1]);
    }

    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 1; i < size; i += 2)
        temp[i] = _mm256_max_epi32(data[i - 1], data[i]); //32 sort finish

    for (int i = 4; i < size; i++)
        temp[i] = _mm256_permute2x128_si256(temp[i], temp[i], 1);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_shuffle_epi32(temp[i], 27);
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_min_epi32(temp[i], temp[idx - i]);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_max_epi32(temp[idx - i], temp[i]);

    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_permute2x128_si256(data[i], data[i + 4], 32);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_permute2x128_si256(data[i - 4], data[i], 49);
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_blend_epi32(temp[i], temp[i + 4], 102);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_blend_epi32(temp[i - 4], temp[i], 153);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_shuffle_epi32(data[i], 78);
    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_min_epi32(data[i], data[i + 4]);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_max_epi32(data[i - 4], data[i]);

    for (int i = 0; i < 4; i++)
        data[i] = _mm256_blend_epi32(temp[i], temp[i + 4], 170);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_blend_epi32(temp[i - 4], temp[i], 85);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_shuffle_epi32(data[i], 27); //177
    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_min_epi32(data[i], data[i + 4]);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_max_epi32(data[i - 4], data[i]);

    for (int i = 0; i < 4; i++)
        data[i] = _mm256_blend_epi32(temp[i], temp[idx - i], 102);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_blend_epi32(temp[idx - i], temp[i], 153);
    for (int i = 0; i < 4; i++)
        temp[i] = _mm256_shuffle_epi32(data[i], 120);
    for (int i = 4; i < size; i++)
        temp[i] = _mm256_shuffle_epi32(data[i], 210); //177
    for (int i = 0; i < 4; i++)
        data[i] = _mm256_min_epi32(temp[i], temp[i + 4]);
    for (int i = 4; i < size; i++)
        data[i] = _mm256_max_epi32(temp[i - 4], temp[i]);

    for (int i = 0; i < size; i += 4)
    {
        temp[i] = _mm256_min_epi32(data[i], data[i + 2]);
        temp[i + 1] = _mm256_min_epi32(data[i + 1], data[i + 3]);
    }
    for (int i = 2; i < size; i += 4)
    {
        temp[i] = _mm256_max_epi32(data[i - 2], data[i]);
        temp[i + 1] = _mm256_max_epi32(data[i - 1], data[i + 1]);
    }

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_min_epi32(temp[i], temp[i + 1]);
    for (int i = 1; i < size; i += 2)
        data[i] = _mm256_max_epi32(temp[i - 1], temp[i]);

    dataOutput<size>(data, temp, output);
    dataOutput<size>(data, temp, output, false);
}

void AVXSort(int *data, size_t length);
