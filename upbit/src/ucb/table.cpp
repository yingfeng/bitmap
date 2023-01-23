#include "fastbit/bitvector.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "fastbit/bitvector.h"
#include <cstdlib>
#include <cstdio>

#include "ucb/table.h"
#include "upbit/util.h"

using namespace ucb;

const ibis::bitvector::word_t UCB_PAD_BITS = (kNumRows/ 1000) * 31;

Table::Table(uint32_t cardinality, uint32_t n_rows) : BaseTable(cardinality, n_rows) {
    total_rows = numRows + UCB_PAD_BITS;
    existence_bitmap.adjustSize(n_rows, total_rows);
    existence_bitmap.compress();
    for (uint32_t i = 0; i < cardinality; ++i) {
        bitmaps[i] = new ibis::bitvector();
        bitmaps[i]->read(getBitmapName(i).c_str());
        if (kEnableFencePointer)
            bitmaps[i]->buildFencePointers();
        bitmaps[i]->adjustSize(numRows, total_rows);
        bitmaps[i]->compress();
    }
    TBC = total_rows;
}

void Table::append(int val) {
    bitmaps[val]->setBit(numRows, 1);
    existence_bitmap.setBit(numRows, 1);
    append_rowids.push_back(numRows);
    numRows += 1;
}

void Table::fastAppend(int to_val) {
    bitmaps[to_val]->appendActive();
    auto size = bitmaps[to_val]->m_vec.size();
    uint32_t buffer[] = {bitmaps[to_val]->m_vec[size - 2], bitmaps[to_val]->m_vec[size - 1]};
    buffer[1] &= ibis::bitvector::MAXCNT;

    // std::cout << std::hex << buffer[0] << std::endl;
    // std::cout << std::hex << buffer[1] << std::endl;

    if (TBC - numRows < buffer[1] * 31) {
        ibis::bitvector::word_t fill0 = ((numRows - (TBC - buffer[1] * 31)) / 31) | (1U << ibis::bitvector::MAXBITS);
        ibis::bitvector::word_t literal = (1U << (numRows % 31));
        ibis::bitvector::word_t fill1 = ((TBC - numRows) / 31) | (1U << ibis::bitvector::MAXBITS);
        if (fill0 > ibis::bitvector::MAXCNT) {
            bitmaps[to_val]->m_vec[size - 1] = fill0;
            bitmaps[to_val]->m_vec.push_back(literal);
            bitmaps[to_val]->m_vec.push_back(fill1);
            // std::cout << std::hex << fill0 << std::endl;
            // std::cout << std::hex << literal << std::endl;
            // std::cout << std::hex << fill1 << std::endl;
        } else {
            bitmaps[to_val]->m_vec[size - 1] = literal;
            bitmaps[to_val]->m_vec.push_back(fill1);
        }
    } else {
        ibis::bitvector::word_t literal = (1U << (numRows % 31)) | buffer[0];
        bitmaps[to_val]->m_vec[size - 2] = literal;
    }
}

void Table::update(uint32_t rowid, int to_val) {
    existence_bitmap.setBit(rowid, 0);
    existence_bitmap.setBit(numRows, 1);
    bitmaps[to_val]->setBit(numRows, 1);
    append_rowids.push_back(rowid);
    numRows += 1;
}

void Table::remove(uint32_t rowid) {
    existence_bitmap.setBit(rowid, 0);
}

int Table::evaluate(uint32_t val, ibis::bitvector &res) {
    if (existence_bitmap.all1s()) {
        res.copy(*bitmaps[val]);
    } else {
        res.copy(existence_bitmap);
        res &= *bitmaps[val];
    }
    auto cnt = res.do_cnt();
    return cnt;
}

int Table::evaluate(uint32_t val, std::vector<uint64_t> &rids) {
    if (existence_bitmap.all1s()) {
        bitmaps[val]->decode(rids);
    } else {
        ibis::bitvector tmp;
        tmp.copy(existence_bitmap);
        tmp &= *bitmaps[val];
        tmp.decode(rids);
    }
    return rids.size();
}

void Table::printMemory() {
    uint64_t bitmap = 0, fence_pointers = 0;
    for (int i = 0; i < cardinality; ++i) {
        // bitmaps[i]->appendActive();
        bitmap += bitmaps[i]->getSerialSize();
        if (kEnableFencePointer) {
            fence_pointers += bitmaps[i]->fencePointers.size() * sizeof(int) * 2;
        }
    }
    std::cout << "M TT " << append_rowids.size() * sizeof(uint32_t) << std::endl;
    std::cout << "M EB " << existence_bitmap.getSerialSize() << std::endl;
    std::cout << "M BM " << bitmap << std::endl;
    std::cout << "M FP " << fence_pointers << std::endl;
}

void Table::printUncompMemory() {
    uint64_t bitmap = 0, fence_pointers = 0;
    for (int i = 0; i < cardinality; ++i) {
        // bitmaps[i]->appendActive();
        bitmaps[i]->decompress();
        bitmap += bitmaps[i]->getSerialSize();
        bitmaps[i]->compress();
        if (kEnableFencePointer) {
            fence_pointers += bitmaps[i]->fencePointers.size() * sizeof(int) * 2;
        }
    }
    std::cout << "UncM TT " << append_rowids.size() * sizeof(uint32_t) << std::endl;
    existence_bitmap.decompress();
    std::cout << "UncM EB " << existence_bitmap.getSerialSize() << std::endl;
    existence_bitmap.compress();
    std::cout << "UncM BM " << bitmap << std::endl;
    std::cout << "UncM FP " << fence_pointers << std::endl;
}
