#include "fastbit/bitvector.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <cstdlib>

#include "inplace/table.h"
#include "upbit/util.h"

#ifdef LINUX
#include <omp.h>
#endif

using namespace inplace;
using namespace std;

Table::Table(uint32_t cardinality, uint32_t n_rows) : BaseTable(cardinality, n_rows) {
    threadPool = new ThreadPool(kNumThreads);
    for (uint32_t i = 0; i < cardinality; ++i) {
        bitmaps[i] = new ibis::bitvector();
        bitmaps[i]->read(getBitmapName(i).c_str());
    }
}

void Table::append(int val) {
    kNumRows += 1;
    bitmaps[val]->setBit(numRows, 1);
    for (int i = 0; i < cardinality; ++i) {
        bitmaps[i]->adjustSize(0, numRows);
    }
}

void Table::update(uint32_t rowid, int to_val) {
    struct timeval before, after;
    int from_val = get_value(rowid);
    if (from_val == to_val || from_val == -1)
        return;

    gettimeofday(&before, NULL);
    bitmaps[from_val]->setBit(rowid, 0);
    bitmaps[to_val]->setBit(rowid, 1);
    gettimeofday(&after, NULL);
    cout << "U SETBIT " << time_diff(before, after) << endl;
    if (kEnableFencePointer) {
        bitmaps[from_val]->buildFencePointers();
        bitmaps[to_val]->buildFencePointers();
    }
}

void Table::remove(uint32_t rowid) {
    auto val = get_value(rowid);
    if (val == -1)
        return;
    bitmaps[val]->setBit(rowid, 0);
    if (kEnableFencePointer) {
        bitmaps[val]->buildFencePointers();
    }
}

int Table::evaluate(uint32_t val, ibis::bitvector &res) {
    // struct timeval before, after;
    // gettimeofday(&before, NULL);
    res.copy(*bitmaps[val]);
    // gettimeofday(&after, NULL);
    // cout << "Q COPY " << time_diff(before, after) << endl;
    // gettimeofday(&before, NULL);
    auto cnt = res.do_cnt();
    // gettimeofday(&after, NULL);
    // cout << "Q DEC " << time_diff(before, after) << endl;
    return cnt;
}

int Table::evaluate(uint32_t val, std::vector<uint64_t> &rids) {
    bitmaps[val]->decode(rids);
    return rids.size();
}

int Table::_get_value(uint32_t rowid, int curValue, volatile bool &flag) {
    int ret = -1;

    if (!flag) {
        auto bit1 = bitmaps[curValue]->getBit(rowid);
        if (flag)
            return ret;
        if (bit1 == 1) {
            flag = true;
            ret = curValue;
        }
    }

    return ret;
}

int Table::get_value(uint32_t rowid) {
    volatile bool flag = false;
    std::vector<std::future<int> > localResults;
    int begin = 0, offset = cardinality / kNumThreads;

    for (int i = 1; i <= kNumThreads; ++i) {
        if (flag)
            break;
        if (i == kNumThreads)
            offset += cardinality % kNumThreads;
        localResults.emplace_back(threadPool->enqueue(
                [this](uint32_t rowid, int value, volatile bool &flag, int begin, int offset) {
                    for (int j = 0; j < offset; ++j) {
                        int res = this->_get_value(rowid, begin + j, flag);
                        if (res != -1)
                            return res;
                    }
                    return -1;
                }, rowid, i, flag, begin, offset));
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
    uint64_t bitmap = 0;
    for (int i = 0; i < cardinality; ++i) {
        // bitmaps[i]->appendActive();
        bitmap += bitmaps[i]->getSerialSize();
    }
    std::cout << "M BM " << bitmap << std::endl;
}

void Table::printUncompMemory() {
    uint64_t bitmap = 0;
    for (int i = 0; i < cardinality; ++i) {
        // bitmaps[i]->appendActive();
        bitmaps[i]->decompress();
        bitmap += bitmaps[i]->getSerialSize();
        bitmaps[i]->compress();
    }
    std::cout << "UncM BM " << bitmap << std::endl;
}