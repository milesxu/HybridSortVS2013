// AVXSort.cpp
//

#include "stdafx.h"
#include "util.h"
#include "CPUSort.h"


std::vector<std::vector<int>> endsInitial(size_t dataLen, int unitLen)
{
    //TODO: if length of all test arrays always be power of 2, the last half
    //is unecessary.
    int chunkNum = dataLen / unitLen + (dataLen % unitLen > 0);
    //int qUnitLen = chunkNum + 1;
    std::vector<std::vector<int>> result;
    std::vector<int> lBound, rBound;
    std::transform(boost::counting_iterator<int>(0),
                   boost::counting_iterator<int>(chunkNum),
                   back_inserter(lBound),
                   [&](int i)
                   {
                       return i * unitLen;
                   });
    result.push_back(lBound);
    std::transform(boost::counting_iterator<int>(0),
                   boost::counting_iterator<int>(chunkNum - 1),
                   back_inserter(result),
                   [&](int i)
                   {
                       return std::vector<int>(chunkNum, 0);
                   });
    std::transform(lBound.begin(), lBound.end(), back_inserter(rBound),
                   [&](int i)
                   {
                       return std::min(i + unitLen, int(dataLen));
                   });
    result.push_back(rBound);
    return result;
}

void intermediateInitial(std::vector<std::vector<int>>& quantile, int row,
                         int unitLen)
{
    std::multimap<int, int> capacity_id;
    std::for_each(boost::counting_iterator<int>(0),
                  boost::counting_iterator<int>(quantile[0].size()), [&](int j)
                  {
                      capacity_id.emplace(quantile[quantile.size() - 1][j]
                                          - quantile[row - 1][j], j);
                  });
    auto n = unitLen;
    auto iter = capacity_id.begin();
    do
    {
        int average = n / capacity_id.size();
        int residue = n % capacity_id.size();
        if (iter->first > average)
        {
            for_each(iter, capacity_id.end(), [&](std::pair<int, int> k)
                     {
                         quantile[row][k.second] = quantile[row - 1][k.second]
                             + average + (residue > 0);
                         if (residue) residue--;
                     });
            iter = capacity_id.end();
        }
        else
        {
            while (iter->first <= average)
            {
                quantile[row][iter->second] = quantile[row - 1][iter->second]
                    + iter->first;
                n -= iter->first;
                iter = capacity_id.erase(iter);
            }
        }
    }
    while (iter != capacity_id.end());
}

void insertItem(int* arr, std::vector<std::vector<int>>& quantile,
                std::multimap<int, int, std::greater<int>>& left,
                std::multimap<int, int>& right, std::vector<int>& pre,
                std::vector<int>& suc, int row, int col)
{
    auto testIdx = quantile[row][col];
    if (testIdx > pre[col])
        left.emplace(arr[testIdx - 1], col);
    if (testIdx < suc[col])
        right.emplace(arr[testIdx], col);
    // TODO: delete all bound justice, or only upper bound?
}

template <typename compare = std::less<int>>
void eraseItem(int key, int row, std::multimap<int, int, compare>& qPair)
{
    auto iter = qPair.lower_bound(key);
    while (iter->second != row) ++iter;
    // && iter != qPair.upper_bound(key)
    qPair.erase(iter);
}

//must have the quantile arrary be initialized, namely each first element is
//assigned.
void quantileCompute(int* arr, int unitLen,
                     std::vector<std::vector<int>>& quantile)
{
    int bound = quantile.size() - 1, chunkNum = quantile[0].size();
    std::vector<int> pre(chunkNum), suc(chunkNum);
    for (auto i = 1; i < bound; i++)
    {
        copy(quantile[i - 1].begin(), quantile[i - 1].end(), pre.begin());
        std::transform(quantile[bound].begin(), quantile[bound].end(),
                       pre.begin(), suc.begin(), [&](int j, int k)
                       {
                           return std::min(j, k + unitLen);
                       });
        intermediateInitial(quantile, i, unitLen);
        std::multimap<int, int, std::greater<int>> left;
        std::multimap<int, int> right;
        std::for_each(boost::counting_iterator<int>(0),
                      boost::counting_iterator<int>(chunkNum), [&](int j)
                      {
                          insertItem(arr, quantile, left, right, pre, suc, i,
                                     j);
                      });
        auto lit = left.begin(), rit = right.begin();
        while (lit->first > rit->first)
        {
            auto lmax = lit->second, rmin = rit->second;
            auto lidx = quantile[i][lmax], ridx = quantile[i][rmin];
            left.erase(lit);
            right.erase(rit);
            if (lidx < suc[lmax])
                eraseItem<>(arr[lidx], lmax, right);
            if (ridx > pre[rmin])
                eraseItem<std::greater<int>>(arr[ridx - 1], rmin, left);
            suc[lmax] = quantile[i][lmax] - 1;
            pre[rmin] = quantile[i][rmin] + 1;
            auto deltaMax = (suc[lmax] - pre[lmax]) >> 1;
            auto deltaMin = (suc[rmin] - pre[rmin]) >> 1;
            auto delta = std::min(deltaMax, deltaMin);
            quantile[i][lmax] = suc[lmax] - delta;
            quantile[i][rmin] = pre[rmin] + delta;
            insertItem(arr, quantile, left, right, pre, suc, i, lmax);
            insertItem(arr, quantile, left, right, pre, suc, i, rmin);
            lit = left.begin();
            rit = right.begin();
        }
    }
}

std::vector<boundary> bulkMove(int* input, int* output, int blockLen, 
    std::vector<std::vector<int>>& quantile)
{
    std::vector<boundary> it_vector(quantile.size() - 1);
    auto chunkNum = quantile[0].size();
    auto factor = ~31;
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (int i = 0; i < quantile.size() - 1; i++)
    {
        auto dst = output + i * blockLen;
        boundary it_pairs, temp_pairs;
        std::multimap<int, std::pair<int*, int*>, std::greater<int>> bound_map;
        align_vector head, tail;
        for (size_t j = 0; j < chunkNum; j++)
        {
            auto h = quantile[i][j], t = quantile[i + 1][j];
            if (h < t)
            {
                auto th = h & factor, tt = t & factor;
                /*if (th + 32 > tt)
                {
                    std::cout << "new bound error!" << std::endl;
                }*/
                if (th != h)
                {
                    th = std::min(th + 32, t);
                    std::copy(input + h, input + th, std::back_inserter(head));
                    std::inplace_merge(head.begin(), head.end() - (th - h),
                        head.end());
                }
                if (tt != t && tt >= h)
                {
                    std::copy(input + tt, input + t, std::back_inserter(tail));
                    std::inplace_merge(tail.begin(), tail.end() - (t - tt),
                        tail.end());
                }
                if (th < tt)
                {
                    //temp_pairs.emplace_back(input + th, input + tt);
                    bound_map.emplace(tt - th, 
                        std::make_pair(input + th, input + tt));
                }
            }
        }
        if (!head.empty())
        {
            bound_map.emplace(head.size(), 
                std::make_pair(head.data(), head.data() + head.size()));
        }
        if (!tail.empty())
        {
            bound_map.emplace(tail.size(),
                std::make_pair(tail.data(), tail.data() + tail.size()));
        }
        for (auto &bound : bound_map)
        {
            auto iter = dst;
            dst = std::copy(bound.second.first, bound.second.second, dst);
            it_pairs.emplace_back(iter, dst);
        }
        it_vector[i] = it_pairs;
    }
    return it_vector;
}

void quantileTest(std::vector<std::vector<int>> &quantile, size_t dataLen, 
    int unitLen, int *ibegin)
{
    if (quantile[0][0] != 0 || quantile.back().back() != dataLen)
    {
        std::cout << "toatal length error!" << std::endl;
        return;
    }
    for (auto i = 0; i < quantile.size() - 1; i++)
    {
        auto n = 0;
        auto lmax = ibegin[quantile[i + 1][0] - 1];
        auto rmin = ibegin[quantile[i + 1][0]];
        for (auto j = 0; j < quantile[i].size(); j++)
        {
            if (!std::is_sorted(ibegin + quantile[i][j], ibegin + quantile[i + 1][j]))
            {
                std::cout << "block bound error at " << i << " " << j << std::endl;
                return;
            }
            auto temp = quantile[i + 1][j] - quantile[i][j];
            if (temp > 0 && temp < 32)
            {
                std::cout << "some block is too short!" << std::endl;
            }
            n += temp;
            lmax = std::max(lmax, ibegin[quantile[i + 1][j] - 1]);
            rmin = std::min(rmin, ibegin[quantile[i + 1][j]]);
        }
        if (n != unitLen)
        {
            std::cout << "unit length error at " << i << " " << std::endl;
            return;
        }
        if (i < quantile.size() - 2 && lmax > rmin)
        {
            std::cout << "quantile compute failed at " << i << std::endl;
            return;
        }
    }
    std::cout << "quantile compute test success!" << std::endl;
    return;
}

//TODO: if needed, mofidy pointer parameter to reference to pointer
//TODO: if there are so many blocks to be merged, more problems must concerned.
//TODO: or, unitlen / chunknum must be greater than 32!!!
void multiwayMerge(int* ibegin, int* iend, int* obegin, int unitLen)
{
    //auto unitLen = getUnitLengthPerCore() >> 2;
    //std::cout << unitLen << std::endl;
    auto quantile = endsInitial(iend - ibegin, unitLen);
    quantileCompute(ibegin, unitLen, quantile);
    //quantileTest(quantile, iend - ibegin, unitLen, ibegin);
    auto boundary_vectors = bulkMove(ibegin, obegin, unitLen, quantile);
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (int i = 0; i < boundary_vectors.size(); i++)
    {
        boundary left, right(boundary_vectors[i]);
        auto rbegin = right.front().first, rend = right.back().second;
        auto lbegin = ibegin + (rbegin - obegin);
        auto lend = ibegin + (rend - obegin);
        /*if (boundary_vectors[i].size() == 1)
        {
            std::cout << "only one block!" << std::endl;
            std::copy(rbegin, rend, lbegin);
            continue;
        }*/
        auto first = true;
        while ((left.size() + right.size()) > 2)
        {
            auto llen = left.size(), rlen = right.size();
            if (llen <= 1)
            {
                if (llen == 1 && rlen & 1)
                {
                    auto output = 
                        left[0].first - (right[0].second - right[0].first);
                    MergeUseAVX(left[0].first, left[0].second, right[0].first,
                        right[0].second, output, true, true);
                    left[0].first = output;
                    right.erase(right.begin());
                    rlen -= 1;
                }
                auto loop = rlen - (rlen & 1 ? 1 : 2);
                auto ltemp = left.empty() ? lend : left[0].first;
                for (auto j = 0; j < loop; j += 2)
                {
                    auto output = ltemp - (right[j].second - right[j].first)
                        - (right[j + 1].second - right[j + 1].first);
                    MergeUseAVX(right[j].first, right[j].second,
                        right[j + 1].first, right[j + 1].second, output, true, true);
                    left.emplace(left.begin(), output, ltemp);
                    ltemp = output;
                }
                right.erase(right.begin(), right.begin() + loop);
                rlen -= loop;
                if (rlen == 2)
                {
                    MergeUseAVX(right[0].first, right[0].second,
                        right[1].first, right[1].second, rbegin, true, true);
                    auto end = rbegin + (right[0].second - right[0].first)
                        + (right[1].second - right[1].first);
                    right.clear();
                    right.emplace_back(rbegin, end);
                }
            }
            else
            {
                if (right[0].first == rbegin)
                {
                    auto loop = llen - (llen & 1);
                    auto rptr = rend;
                    for (auto j = 0; j < loop; j += 2)
                    {
                        auto rtemp = rptr - (left[j].second - left[j].first)
                            - (left[j + 1].second - left[j + 1].first);
                        MergeUseAVX(left[j].first, left[j].second,
                            left[j + 1].first, left[j + 1].second, rtemp, true, true);
                        right.emplace(right.begin() + 1, rtemp, rptr);
                        rptr = rtemp;
                    }
                    left.erase(left.begin(), left.begin() + loop);
                    if (!left.empty())
                    {
                        auto lptr = 
                            left[0].first - (right[0].second - right[0].first);
                        MergeUseAVX(left[0].first, left[0].second,
                            right[0].first, right[0].second, lptr, true, true);
                        left[0].first = lptr;
                        right.erase(right.begin());
                    }
                }
                else
                {
                    auto rptr = right[0].first;
                    if (llen & 1)
                    {
                        rptr -= (left[0].second - left[0].first);
                        MergeUseAVX(right[0].first, right[0].second,
                            left[0].first, left[0].second, rptr, true, true);
                        right[0].first = rptr;
                        left.erase(left.begin());
                    }
                    for (auto j = 0; j < left.size(); j += 2)
                    {
                        auto rtemp = rptr - (left[j].second - left[j].first)
                            - (left[j + 1].second - left[j + 1].first);
                        MergeUseAVX(left[j].first, left[j].second,
                            left[j + 1].first, left[j + 1].second, rtemp, true, true);
                        right.emplace(right.begin(), rtemp, rptr);
                        rptr = rtemp;
                    }
                    left.clear();
                }
            }
            first = false;
        }
        if (left.empty())
        {
            MergeUseAVX(right[0].first, right[0].second, right[1].first,
                right[1].second, lbegin, true, true);
        }
        else
        {
            MergeUseAVX(left[0].first, left[0].second, right[0].first,
                right[0].second, lbegin, true, true);
        }
    }
}

void AVXSort(int* begin, int* end)
{
    size_t length = end - begin;
    const auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
    CoreSortStage1(begin, temp, length);
    _mm_free(temp);
}

void ompAVXSort(int* begin, int* end)
{
    //TODO: is there a need to use size_t?
    int length = end - begin;
    const auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
    //TODO: unify this variable in caller functions!
    auto byteLen = length >> 3;
    //size_t unitLen = 64;
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (auto i = 0; i < length; i += byteLen)
    {
        CoreSortStage1(begin + i, temp + i, byteLen);
        //std::cout << std::is_sorted(begin + i, begin + i + byteLen);
    }
    multiwayMerge(begin, end, temp, byteLen);
    _mm_free(temp);
}

void recursiveMultiwayMerge(int *begin, int *end)
{
    const auto log_chunk = 5, limit = 1 << 20;
    auto length = end - begin;
    auto byteLen = length;
    while (byteLen >= limit)
    {
        byteLen >>= log_chunk;
    }
    const auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (auto i = 0; i < length; i += byteLen)
    {
        CoreSortStage1(begin + i, temp + i, byteLen);
    }
    while (byteLen < length)
    {
        auto nextLen = byteLen << log_chunk;
        for (auto i = 0; i < length; i += nextLen)
            multiwayMerge(begin + i, begin + i + nextLen, temp + i, byteLen);
        byteLen = nextLen;
    }
    _mm_free(temp);
}