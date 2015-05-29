// AVXSort.cpp
//

#include "stdafx.h"
#include "CPUSort.h"


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

//must have the quantile arrary be initialized, namely each first element is
//assigned.
//each row is a bound(up or down) for a quantile, each column contains all the
//quantiles of a chunk. so length of each row is chunkNum, and length of each
//column is qUnitLen.
//TODO: may be use a unified malloc function and buffer.
void quantileCompute(int *data, size_t *quantile, size_t unitLen,
                     size_t chunkNum, size_t qUnitLen, size_t dataLen)
{
    if (quantile[0] == 0 && quantile[chunkNum] == 0)
        return;
    size_t *bound = quantile + chunkNum * qUnitLen;
    size_t *pre = (size_t *)_mm_malloc(chunkNum * sizeof(size_t), 32);
    size_t *nxt = (size_t *)_mm_malloc(chunkNum * sizeof(size_t), 32);
    size_t *cur = quantile + chunkNum;
    for (size_t i = 1; i < qUnitLen; i++) {
        std::copy(cur - chunkNum, cur, cur);
        std::copy(cur, cur + chunkNum, pre);
        size_t n = unitLen;
        std::vector<size_t> idxs;
        size_t j = 0;
        std::for_each(nxt, nxt + chunkNum, [&](size_t &idx){
                idx = std::min(bound[j], cur[j++] + chunkNum);
            });
        j = 0;
        std::for_each(cur, cur + chunkNum, [&](size_t idx){
                if (idx < bound[j]) idxs.push_back(j++);
            });
        
        //initialize quantiles in cur row. try get average.
        do
        {
            size_t average = n / idxs.size();
            size_t residue = n % idxs.size();
            auto k = idxs.begin();
            do
            {
                j = *k;
                size_t capacity = bound[j] - cur[j];
                if (capacity > average)
                {
                    cur[j] += average + (residue > 0);
                    n -= average + (residue > 0);
                    if (residue) residue--;
                }
                else
                {
                    cur[j] += capacity;
                    n -= capacity;
                }
                //if (cur[j] == bound[j])
                //k = idxs.erase(k);
                //else
                k++;
            } while (k != idxs.end());
            std::remove_if(idxs.begin(), idxs.end(), [&](size_t p){
                    return cur[p] == bound[p];
                });
        } while (n);
        
        //begin to compute quantile
        do
        {
            const int *lmax = NULL, *rmin = NULL;
            size_t lmaxchk, rminchk;
            for (j = 0; j < chunkNum; j++) {
                size_t testIdx = cur[j];
                if (testIdx > pre[j] && (!lmax || *lmax < data[testIdx - 1]))
                {
                    lmax = data + testIdx - 1;
                    lmaxchk = j;
                }
                if (testIdx < nxt[j] && (!rmin || *rmin > data[testIdx]))
                {
                    rmin = data + testIdx;
                    rminchk = j;
                }
            }
            if (!lmax || !rmin || lmaxchk == rminchk || *lmax < *rmin ||
                (*lmax == *rmin && lmaxchk < rminchk))
                break;
            nxt[lmaxchk] = cur[lmaxchk] - 1;
            pre[rminchk] = cur[rminchk] + 1;
            size_t deltaMax = (nxt[lmaxchk] - pre[lmaxchk]) >> 1;
            size_t deltaMin = (nxt[rminchk] - pre[rminchk]) >> 1;
            size_t delta = std::min(deltaMax, deltaMin);
            cur[lmaxchk] = nxt[lmaxchk] - delta;
            cur[rminchk] = pre[rminchk] + delta;
        } while (true);
        cur += chunkNum;
    }
    _mm_free(pre);
    _mm_free(nxt);
}
