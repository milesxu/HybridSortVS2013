// AVXSort.cpp
//

#include "stdafx.h"


template<int size>
void storeData(int *&output, __m256i (&data)[size])
{
    for (int i = 0; i < size; ++i)
    {
        _mm256_store_si256((__m256i *)output, data[i]);
        output += rUnit;
    }
}

void AVXSort(int *data, size_t length)
{
    __m256i rBuffer[bufferSize];
    //TODO: may add more conditions such as length < bufferSize or
    //length is not divisible by bufferSize
    __m256i *input = (__m256i *)data;
    for (int i = 0; i < length / rUnit; i += 2)
    {
        //rBuffer[i] = _mm256_load_si256(input);
        //rBuffer[i] = input[i];
        rBuffer[i] = _mm256_min_epi32(input[i], input[i + 1]);
    }
    for (int i = 1; i < length / rUnit; i += 2)
    {
        rBuffer[i] = _mm256_max_epi32(input[i - 1], input[i]);
    }
    // for (int i = 0; i < length / bufferSize; ++i)
    // {
    //     for (int j = 0; j < bufferSize / rUnit; ++j)
    //     {

    //     }
    // }
}

/*template<int size>
void bitonicSort(__m256i (&data)[size], int *&output)
{
    __m256i temp[size];
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(data[i], data[i + 1]);

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_shuffle_epi32(temp[i + 1], 177);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_min_epi32(temp[i], data[i]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(temp[i], data[i]);

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 170);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 85);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_shuffle_epi32(temp[i], 177);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(data[i], data[i + 1]);

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_shuffle_epi32(temp[i + 1], 27);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_min_epi32(temp[i], data[i]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(temp[i], data[i]);

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 170);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 85);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_shuffle_epi32(temp[i], 177);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_min_epi32(data[i], data[i + 1]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(data[i], data[i + 1]);

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_shuffle_epi32(temp[i + 1], 27);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_min_epi32(temp[i], data[i]);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_max_epi32(temp[i], data[i]);
    for (int i = 0; i < size; i += 2)
        data[i] = temp[i + 1];

    for (int i = 0; i < size; i += 2)
        data[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 204);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(data[i + 1], temp[i + 1], 51);
    for (int i = 0; i < size; i += 2)
        data[i + 1] = _mm256_shuffle_epi32(temp[i], 78);
    for (int i = 0; i < size; i += 2)
        temp[i] = _mm256_blend_epi32(data[i], data[i + 1], 204);
    for (int i = 0; i < size; i += 2)
        temp[i + 1] = _mm256_blend_epi32(data[i], data[i + 1], 51);
    for (int i = 0; i < size; i++)
    {
        _mm256_store_si256((__m256i *)output, data[i + 1]);
        output += rUnit;
        _mm256_store_si256((__m256i *)output, temp[i]);
        output += rUnit;
    }
}*/
