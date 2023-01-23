#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

#include "fastbit/bitvector.h"
#include "table.h"

using namespace upbit;
using namespace std;

static const ibis::bitvector::word_t kNumPadBits = (kNumRows/ 1000) * 31;

Table::Table(uint32_t cardinality, uint32_t numRows) : BaseTable(cardinality, numRows), updateBitmaps(cardinality) {
    curtNumRows = numRows + kNumPadBits;

    for (uint32_t i = 0; i < cardinality; ++i) {
        bitmaps[i] = new ibis::bitvector();
        bitmaps[i]->read(getBitmapName(i).c_str());
        // append pad bits
        bitmaps[i]->adjustSize(0, curtNumRows);
        bitmaps[i]->buildFencePointers();
        updateBitmaps[i] = new ibis::bitvector();
        updateBitmaps[i]->adjustSize(0, curtNumRows);
    }

    threadPool = new ThreadPool(kNumThreads);
}

void Table::append(int val) {
    bitmaps[val]->setBit(curtNumRows, 1);
    numRows += 1;
}

void Table::update(unsigned int rowid, int to_val) {
    int from_val = getValue(rowid);
    if (from_val == -1 || to_val == from_val)
        return;

    updateBitmaps[to_val]->flipBit(rowid);
    updateBitmaps[from_val]->flipBit(rowid);
    // updateBitmaps[to_val]->setBit(rowid, !updateBitmaps[to_val]->getBit(rowid));
    // updateBitmaps[from_val]->setBit(rowid, !updateBitmaps[from_val]->getBit(rowid));
}

void Table::remove(unsigned int rowid) {
    int val = getValue(rowid);
    if (val == -1)
        return;

    updateBitmaps[val]->flipBit(rowid);
    // updateBitmaps[val]->setBit(rowid, !updateBitmaps[val]->getBit(rowid));
}

int Table::evaluate(uint32_t val, ibis::bitvector &res) {
    auto cnt = updateBitmaps[val]->cnt();

    if (cnt == 0) {
        res.copy(*bitmaps[val]);
    } else {
        res.copy(*updateBitmaps[val]);
        res ^= *bitmaps[val];
    }

    if (cnt > kMergeThreshold) {
        bitmaps[val]->fencePointers.clear();
        bitmaps[val]->copy(res);
        bitmaps[val]->adjustSize(0, curtNumRows);
        bitmaps[val]->buildFencePointers();
        updateBitmaps[val]->clear();
        updateBitmaps[val]->adjustSize(0, curtNumRows);
    }

    return res.do_cnt();
}

int Table::evaluate(uint32_t val, std::vector<uint64_t> &rids) {
    auto cnt = updateBitmaps[val]->cnt();

    if (cnt == 0) {
        bitmaps[val]->decode(rids);
    } else {
        if (cnt > kMergeThreshold) {
            *updateBitmaps[val] ^= *bitmaps[val];
            bitmaps[val]->swap(*updateBitmaps[val]);
            bitmaps[val]->fencePointers.clear();
            bitmaps[val]->decode(rids);
            bitmaps[val]->adjustSize(0, curtNumRows);
            bitmaps[val]->buildFencePointers();
            updateBitmaps[val]->clear();
            updateBitmaps[val]->adjustSize(0, curtNumRows);
        } else {
            bitmaps[val]->decode(rids, *updateBitmaps[val]);
        }
    }

    return rids.size();
}

inline int Table::_getValue(uint32_t rowid, int curValue, bool *found) {
    int ret = -1;

    if (*access_once(found) == false) {
        auto bit1 = bitmaps[curValue]->getBitWithIndex(rowid);
        auto bit2 = updateBitmaps[curValue]->getBit(rowid);
        if ((bit1 ^ bit2) == 1) {
            *found = true;
            ret = curValue;
        }
    }

    return ret;
}

int Table::getValue(uint32_t rowid) {
    bool found = false;
    std::vector<std::future<int> > localResults;
    int begin = 0, offset = cardinality / kNumThreads;

    for (int i = 1; i <= kNumThreads; ++i) {
        if (found)
            break;
        if (i == kNumThreads)
            offset += cardinality % kNumThreads;
        localResults.emplace_back(threadPool->enqueue(
                [this](uint32_t rowid, int value, bool *found, int begin, int offset) {
                    for (int j = 0; j < offset; ++j) {
                        int res = this->_getValue(rowid, begin + j, found);
                        if (res != -1)
                            return res;
                    }
                    return -1;
                }, rowid, i, &found, begin, offset));
        begin += offset;
    }

    int ret = -1;
    for (auto &&result : localResults) {
        int res = result.get();
        if (res != -1) {
            ret = res;
            break;
        }
    }

    return ret;
}

void Table::printMemory() {
    uint64_t bitmap = 0, updateable_bitmap = 0, fence_pointers = 0;
    for (int i = 0; i < cardinality; ++i) {
        bitmap += bitmaps[i]->getSerialSize();
        fence_pointers += bitmaps[i]->fencePointers.size() * sizeof(int) * 2;
        updateable_bitmap += updateBitmaps[i]->getSerialSize();
    }
    std::cout << "M FP " << fence_pointers << std::endl;
    std::cout << "M UB " << updateable_bitmap << std::endl;
    std::cout << "M BM " << bitmap << std::endl;
}

void Table::printUncompMemory() {
    uint64_t bitmap = 0, updateable_bitmap = 0, fence_pointers = 0;
    for (int i = 0; i < cardinality; ++i) {
        // bitmaps[i]->appendActive();
        bitmaps[i]->decompress();
        bitmap += bitmaps[i]->getSerialSize();
        bitmaps[i]->compress();
        updateBitmaps[i]->decompress();
        updateable_bitmap += updateBitmaps[i]->getSerialSize();
        updateBitmaps[i]->compress();
        fence_pointers += bitmaps[i]->fencePointers.size() * sizeof(int) * 2;
    }
    std::cout << "UncM FP " << fence_pointers << std::endl;
    std::cout << "UncM UB " << updateable_bitmap << std::endl;
    std::cout << "UncM BM " << bitmap << std::endl;
}

int Table::range(uint32_t start, uint32_t range) {
    ibis::bitvector res;
    res.set(0, curtNumRows);
    if (kRangeAlgoName == "naive") {
        for (uint32_t i = 0; i < range; ++i) {
            res |= *(bitmaps[start + i]);
        }
    } else if (kRangeAlgoName == "pq") {
        typedef std::pair<ibis::bitvector *, bool> _elem;
        // put all bitmaps in a priority queue
        std::priority_queue<_elem> que;
        _elem op1, op2, tmp;
        tmp.first = 0;

        // populate the priority queue with the original input
        for (uint32_t i = 0; i < range; ++i) {
            op1.first = bitmaps[start + i];
            op1.second = false;
            que.push(op1);
        }

        while (!que.empty()) {
            op1 = que.top();
            que.pop();
            if (que.empty()) {
                res.copy(*(op1.first));
                if (op1.second) delete op1.first;
                break;
            }

            op2 = que.top();
            que.pop();
            tmp.second = true;
            tmp.first = *(op1.first) | *(op2.first);

            if (op1.second)
                delete op1.first;
            if (op2.second)
                delete op2.first;
            if (!que.empty()) {
                que.push(tmp);
                tmp.first = 0;
            }
        }
        if (tmp.first != 0) {
            if (tmp.second) {
                res |= *(tmp.first);
                delete tmp.first;
                tmp.first = 0;
            } else {
                res |= *(tmp.first);
            }
        }
    } else {
        auto end = start + range;
        while (start < end && bitmaps[start] == 0)
            ++start;
        if (start < end) {
            res |= *(bitmaps[start]);
            ++start;
        }
        res.decompress();
        for (uint32_t i = start; i < end; ++i) {
            res |= *(bitmaps[i]);
        }
    }
    return res.do_cnt();
}