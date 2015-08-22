// AVXSort.cpp
//

#include "stdafx.h"
#include "util.h"
#include "CPUSort.h"


//must have the quantile arrary be initialized, namely each first element is
//assigned.
//each row is a bound(up or down) for a quantile, each column contains all the
//quantiles of a chunk. so length of each row is chunkNum, and length of each
//column is qUnitLen.
//TODO: may be use a unified malloc function and buffer.
void quantileCompute(int* data, size_t* quantile, size_t unitLen,
                     size_t chunkNum, size_t qUnitLen, size_t dataLen)
{
    /*if (quantile[0] == 0 && quantile[chunkNum] == 0)
	  return;*/
    //size_t *bound = quantile + chunkNum * qUnitLen;
    auto bound = quantile + chunkNum * (qUnitLen - 1);
    auto pre =
        static_cast<size_t *>(_mm_malloc(chunkNum * sizeof(size_t), 32));
    auto nxt =
        static_cast<size_t *>(_mm_malloc(chunkNum * sizeof(size_t), 32));
    auto cur = quantile + chunkNum;
    //may be the bound is qUnitLen - 1?
    for (size_t i = 1; i < qUnitLen; i++)
    {
        std::copy(cur - chunkNum, cur, cur);
        std::copy(cur, cur + chunkNum, pre);
        auto n = unitLen;
        std::vector<size_t> idxs;
        //may be add unitLen?
        std::transform(bound, bound + chunkNum, cur, nxt,
                       [&](size_t k, size_t j)
                       {
                           return std::min(k, j + chunkNum);
                       });
        std::multimap<size_t, size_t> capacity_id;
        std::transform(boost::counting_iterator<size_t>(0),
                       boost::counting_iterator<size_t>(chunkNum),
                       std::inserter(capacity_id, capacity_id.begin()),
                       [&](size_t j)
                       {
                           return std::make_pair(bound[j] - cur[j], j);
                       });

        //initialize quantiles in cur row. try get average.
        auto iter = capacity_id.begin();
        do
        {
            auto average = n / capacity_id.size();
            auto residue = n % capacity_id.size();
            if (iter->first > average)
            {
                std::for_each(iter, capacity_id.end(),
                              [&](std::pair<size_t, size_t> k)
                              {
                                  cur[k.second] += average + (residue > 0);
                                  if (residue) residue--;
                              });
                iter = capacity_id.end();
            }
            else
            {
                while (iter->first <= average)
                {
                    cur[iter->second] += iter->first;
                    n -= iter->first;
                    iter = capacity_id.erase(iter);
                }
            }
        }
        while (iter != capacity_id.end());

        //begin to compute quantile
        do
        {
            const int *lmax = nullptr, *rmin = nullptr;
            size_t lmaxchk = -1, rminchk = -1;
            for (size_t j = 0; j < chunkNum; j++)
            {
                auto testIdx = cur[j];
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
            //if *lmax == *rmin, then any item can be *lmax or *rmin, how shall
            //we partition these equal items?
            //last judgement ensure the "upper" item of equal item bounds to be
            //included in left chunk, the other is in right chunk.
            if (!lmax || !rmin || lmaxchk == rminchk || *lmax < *rmin ||
                (*lmax == *rmin && lmaxchk < rminchk))
                break;
            nxt[lmaxchk] = cur[lmaxchk] - 1;
            pre[rminchk] = cur[rminchk] + 1;
            auto deltaMax = (nxt[lmaxchk] - pre[lmaxchk]) >> 1;
            auto deltaMin = (nxt[rminchk] - pre[rminchk]) >> 1;
            auto delta = std::min(deltaMax, deltaMin);
            cur[lmaxchk] = nxt[lmaxchk] - delta;
            cur[rminchk] = pre[rminchk] + delta;
        }
        while (true);
        cur += chunkNum;
    }
    _mm_free(pre);
    _mm_free(nxt);
}

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
//#pragma omp parallel for num_threads(8) schedule(dynamic)
    for (int i = 0; i < quantile.size() - 1; i++)
    {
        auto dst = output + i * blockLen;
        boundary it_pairs, temp_pairs;
        align_vector head, tail;
        auto n = 0;
        for (size_t j = 0; j < chunkNum; j++)
        {
            auto h = quantile[i][j], t = quantile[i + 1][j];
            std::cout << std::is_sorted(input + h, input + t) << std::endl;
            if (h < t)
            {
                //std::cout << t - h << " " << t << " " << h << std::endl;
                n += (t - h);
                auto th = h & factor, tt = t & factor;
                if (th != h)
                {
                    th += 32;
                    std::copy(input + h, input + th, std::back_inserter(head));
                    std::inplace_merge(head.begin(), head.end() - (th - h),
                        head.end());
                }
                if (tt != t)
                {
                    std::copy(input + tt, input + t, std::back_inserter(tail));
                    std::inplace_merge(tail.begin(), tail.end() - (t - tt),
                        tail.end());
                }
                temp_pairs.emplace_back(input + th, input + tt);
            }
        }
        /*std::cout << head.size() << " " << tail.size() << std::endl;
        std::cout << n << std::endl;
        if (!head.empty())
            std::cout << std::is_sorted(head.begin(), head.end()) << std::endl;
        else
            std::cout << "empty head" << std::endl;
        if (!tail.empty())
            std::cout << std::is_sorted(tail.begin(), tail.end()) << std::endl;
        else
        {
            std::cout << "empty tail" << std::endl;
        }*/
        std::multimap<size_t, size_t, std::greater<int>> dis_map;
        auto hid = -1, tid = -1;
        for (size_t j = 0; j < temp_pairs.size(); j++)
        {
            auto temp = temp_pairs[j].second - temp_pairs[j].first;
            if (hid < 0 && !head.empty() 
                && head.back() <= *temp_pairs[j].first)
            {
                hid = j;
                temp += head.size();
            }
            if (tid < 0 && !tail.empty() 
                && tail.front() >= *temp_pairs[j].second)
            {
                tid = j;
                temp += tail.size();
            }
            dis_map.emplace(temp, j);
        }
        for (auto &dis : dis_map)
        {
            auto iter = dst;
            if (hid == dis.second)
            {
                //dst = std::move(head.begin(), head.end(), dst);
                dst = CopyUseAVX(head.data(), head.data() + head.size(), dst);
                hid = -1;
            }
            auto pair = temp_pairs[dis.second];
            //dst = std::move(pair.first, pair.second, dst);
            dst = CopyUseAVX(pair.first, pair.second, dst);
            if (tid == dis.second)
            {
                //dst = std::move(tail.begin(), tail.end(), dst);
                dst = CopyUseAVX(tail.data(), tail.data() + tail.size(), dst);
                tid = -1;
            }
            it_pairs.emplace_back(iter, dst);
            /*std::cout << iter - output << " " << dst - iter << " "
                << std::is_sorted(iter, dst) << std::endl;*/
        }
        it_vector[i] = it_pairs;
        /*std::cout << it_vector[i].back().second - it_vector[i].front().first
            << std::endl;*/
    }
    return it_vector;
}

//TODO: use an array to store upper bounds of sorted chunks.
//mUnitLen is the length of blocks after multiway merge, chunkLen is length
//of sorted chunks that to be merged, munitlen <= chunkLen
//It is best to be sure that chunkNum is power of 2 and grater or equal than 8.
void multiwayMerge(int* data, int* dtemp, size_t dataLen, int chunkLen,
                   int mUnitLen)
{
    int chunkNum = dataLen / chunkLen;
    int qUnitLen = dataLen / mUnitLen + 1;
    auto len = chunkNum * qUnitLen;
    auto quantile = static_cast<size_t *>(_mm_malloc(len * sizeof(size_t), 16));
    std::fill(quantile, quantile + len, size_t(0));
    std::transform(boost::counting_iterator<int>(0),
                   boost::counting_iterator<int>(chunkNum), quantile,
                   [&](int i)
                   {
                       return i * chunkLen;
                   });
    std::transform(boost::counting_iterator<int>(1),
                   boost::counting_iterator<int>(chunkNum + 1),
                   quantile + chunkNum * (qUnitLen - 1), [&](int i)
                   {
                       return i * chunkLen;
                   });
    quantileCompute(data, quantile, mUnitLen, chunkNum, qUnitLen, dataLen);
    _mm_free(quantile);
}

//TODO: if needed, mofidy pointer parameter to reference to pointer
void multiwayMerge(int* ibegin, int* iend, int* obegin)
{
    auto unitLen = getUnitLengthPerCore() >> 2;
    auto quantile = endsInitial(iend - ibegin, unitLen);
    quantileCompute(ibegin, unitLen, quantile);
    auto boundary_vectors = bulkMove(ibegin, obegin, unitLen, quantile);
    /*auto lpoint = ibegin;
    for (auto i : boundary_vectors)
    {
        std::sort(i.front().first, i.back().second);
        lpoint = std::copy(i.front().first, i.back().second, lpoint);
    }*/
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (int i = 0; i < boundary_vectors.size(); i++)
    {
        boundary left, right(boundary_vectors[i]);
        /*if (i == 2)
        {*/
            /*for (auto jb : right)
                std::cout << jb.second - jb.first << " ";
            std::cout << std::endl;*/
        //}
        auto rbegin = right.front().first, rend = right.back().second;
        //std::cout << rbegin - obegin << " " << rend - rbegin << std::endl;
        auto lbegin = ibegin + (rbegin - obegin);
        auto lend = ibegin + (rend - obegin);
        auto first = true;
        while ((left.size() + right.size()) > 2)
        {
            auto llen = left.size(), rlen = right.size();
            //std::cout << llen << " " << rlen << std::endl;
            if (llen <= 1)
            {
                if (llen == 1 && rlen & 1)
                {
                    auto output = 
                        left[0].first - (right[0].second - right[0].first);
                    MergeUseAVX(left[0].first, left[0].second, right[0].first,
                        right[0].second, output, true, true);
                    //std::cout << std::is_sorted(output, left[0].second) << " ";
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
                    //std::cout << std::is_sorted_until(output, ltemp) - output << " ";
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
                    //std::cout << std::is_sorted_until(rbegin, end) - rbegin << " ";
                    right.clear();
                    right.emplace_back(rbegin, end);
                }
            }
            else
            {
                if (right[0].first == rbegin)
                {
                    //std::cout << "reach here!" << std::endl;
                    auto loop = llen - (llen & 1);
                    auto rptr = rend;
                    for (auto j = 0; j < loop; j += 2)
                    {
                        auto rtemp = rptr - (left[j].second - left[j].first)
                            - (left[j + 1].second - left[j + 1].first);
                        MergeUseAVX(left[j].first, left[j].second,
                            left[j + 1].first, left[j + 1].second, rtemp, true, true);
                        //std::cout << std::is_sorted(rtemp, rptr) << " ";
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
                        //std::cout << std::is_sorted(lptr, left[0].second) << " ";
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
                        //std::cout << std::is_sorted(rptr, right[0].second) << " ";
                        right[0].first = rptr;
                        left.erase(left.begin());
                    }
                    for (auto j = 0; j < left.size(); j += 2)
                    {
                        auto rtemp = rptr - (left[j].second - left[j].first)
                            - (left[j + 1].second - left[j + 1].first);
                        MergeUseAVX(left[j].first, left[j].second,
                            left[j + 1].first, left[j + 1].second, rtemp, true, true);
                        //std::cout << std::is_sorted(rtemp, rptr) << " ";
                        right.emplace(right.begin(), rtemp, rptr);
                        rptr = rtemp;
                    }
                    left.clear();
                }
            }
            first = false;
        }
        std::cout << std::endl;
        if (left.empty())
        {
            MergeUseAVX(right[0].first, right[0].second, right[1].first,
                right[1].second, lbegin, true, true);
        }
        else
        {
            MergeUseAVX(left[0].first, left[0].second, right[0].first,
                right[0].second, lbegin, true, true);
            //std::cout << "reach here!" << std::endl;
        }
        //std::cout << i << " " << std::is_sorted(lbegin, lend) << std::endl;
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
    int byteLen = getUnitLengthPerCore() >> 2;
    //size_t unitLen = 64;
    //auto nthreads = 4;
#pragma omp parallel for num_threads(THREADS) schedule(dynamic)
    for (auto i = 0; i < length; i += byteLen)
    {
        CoreSortStage1(begin + i, temp + i, byteLen);
    }
    multiwayMerge(begin, end, temp);
    _mm_free(temp);
}
