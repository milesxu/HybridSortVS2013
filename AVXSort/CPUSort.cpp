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
void quantileCompute(int *data, size_t *quantile, size_t unitLen,
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
    for (size_t i = 1; i < qUnitLen; i++) {
        std::copy(cur - chunkNum, cur, cur);
        std::copy(cur, cur + chunkNum, pre);
        auto n = unitLen;
        std::vector<size_t> idxs;
        //size_t j = 0;
		  /*std::for_each(nxt, nxt + chunkNum, [&](size_t &idx){
                idx = std::min(bound[j], cur[j++] + chunkNum);
            });*/
		//may be add unitLen?
		std::transform(bound, bound + chunkNum, cur, nxt,
					   [&](size_t k, size_t j){
						   return std::min(k, j + chunkNum);
					   });
        /*size_t j = 0;
        std::for_each(cur, cur + chunkNum, [&](size_t idx){
                if (idx < bound[j]) idxs.push_back(j++);
				});*/
		/*std::copy_if(boost::counting_iterator<size_t>(0),
					 boost::counting_iterator<size_t>(chunkNum), idxs,
					 [&](size_t i){
						 return cur[i] < bound[i];
						 });*/
		/*size_t j = 0;
		std::for_each(boost::counting_iterator<size_t>(0),
					  boost::counting_iterator<size_t>(chunkNum),
					  [&](size_t i){
						  if (cur[i] < bound[i])
						  {
							  idxs.push_back(i);
						  }
					  });*/
		std::multimap<size_t, size_t> capacity_id;
		std::transform(boost::counting_iterator<size_t>(0),
					   boost::counting_iterator<size_t>(chunkNum),
					   std::inserter(capacity_id, capacity_id.begin()),
					   [&](size_t j){
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
							  [&](std::pair<size_t, size_t> k){
								  cur[k.second] += average + (residue > 0);
								  if (residue) residue--;
							  });
				iter = capacity_id.end();
			}
			else
			{
				while (iter->first <= average) {
					cur[iter->second] += iter->first;
					n -= iter->first;
					iter = capacity_id.erase(iter);
				}
			}
		} while (iter != capacity_id.end());
        /*do
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
            idxs.erase(std::remove_if(idxs.begin(), idxs.end(), [&](size_t p){
 						return cur[p] == bound[p];}),
				idxs.end());
				} while (n);*/
        
        //begin to compute quantile
        do
        {
            const int *lmax = nullptr, *rmin = nullptr;
            size_t lmaxchk = -1, rminchk = -1;
            for (size_t j = 0; j < chunkNum; j++) {
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
        } while (true);
        cur += chunkNum;
    }
    _mm_free(pre);
    _mm_free(nxt);
}

void AVXSort(int *begin, int *end)
{
	size_t length = end - begin;
    const auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
    CoreSortStage1(begin, temp, length);
    _mm_free(temp);
}

void ompAVXSort(int *begin, int *end)
{
	//TODO: is there a need to use size_t?
	int length = end - begin;
    const auto temp = static_cast<int *>(_mm_malloc(length * sizeof(int), 32));
	//TODO: unify this variable in caller functions!
    int byteLen = getUnitLengthPerCore() >> 2;
    //size_t unitLen = 64;
    //auto nthreads = 4;
#pragma omp parallel for num_threads(8) schedule(dynamic)
	for (auto i = 0; i < length; i += byteLen) {
		CoreSortStage1(begin + i, temp + i, byteLen);
	}
	_mm_free(temp);
}

void blockMerge(int *data, int *dtemp, size_t *qLeft, size_t *qRight,
				int *unalign, int chunkNum)
{
	std::vector<int, boost::alignment::aligned_allocator<int, 32>> head, tail;
	int headId = -1, tailId = -1;
	std::vector<std::pair<size_t, size_t>> bounds;
	/*std::for_each(boost::counting_iterator<size_t>(0),
	  boost::counting_iterator<size_t>(chunkNum), [&](size_t i){*/
	for(size_t i = 0; i < chunkNum; ++i){
		if (qLeft[i] < qRight[i]){
			size_t hd = qLeft[i], tl = qRight[i];
			if (unalign[i]){
				hd += 32 - unalign[i];
				int len = head.size();
				std::copy(qLeft[i], hd, std::back_inserter(head));
				if (len)
					std::inplace_merge(head.begin(), head.begin() + len,
									   head.end());
				if (headId < 0 || *(data + hd - 1)
					> *(data + bounds[headId].first - 1))
					headId = len;
			}
			unalign[i] = (tl - hd) & ~31;
			if (unalign[i]){
				tl -= unalign[i];
				int len = tail.size();
				std::copy(tl, qRight[i], std::back_inserter(tail));
				if (len)
					std::inplace_merge(tail.begin(), tail.begin() + len,
									   tail.end());
				if (tailId < 0 || *(data + tl + 1)
					< *(data + bounds[tailId].second + 1))
					tailId = len;
			}
			bounds.push_back(std::make_pair(hd, tl));
		}
	}
}

//TODO: use an array to store upper bounds of sorted chunks.
//mUnitLen is the length of blocks after multiway merge, chunkLen is length
//of sorted chunks that to be merged, munitlen <= chunkLen
//It is best to be sure that chunkNum is power of 2 and grater or equal than 8.
void multiwayMerge(int *data, int *dtemp, size_t dataLen, int chunkLen,
				   int mUnitLen)
{
	int chunkNum = dataLen / chunkLen;
	int qUnitLen = dataLen / mUnitLen + 1;
	int len = chunkNum * qUnitLen;
	size_t *quantile = (size_t *)_mm_malloc(len * sizeof(size_t), 16);
	std::fill(quantile, quantile + len, size_t(0));
	std::transform(boost::counting_iterator<int>(0),
				   boost::counting_iterator<int>(chunkNum), quantile, [&](int i){
					   return i * chunkLen;
				   });
	std::transform(boost::counting_iterator<int>(1),
				   boost::counting_iterator<int>(chunkNum + 1),
				   quantile + chunkNum * (qUnitLen - 1), [&](int i){
					   return i * chunkLen;
				   });
	quantileCompute(data, quantile, mUnitLen, chunkNum, qUnitLen, dataLen);
	_mm_free(quantile);
}
