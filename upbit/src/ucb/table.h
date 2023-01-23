#ifndef UCB_TABLE_H_
#define UCB_TABLE_H_

#include "fastbit/bitvector.h"
#include "upbit/table.h"

#include <string>
#include <vector>
#include <queue>

namespace ucb {

    class Table : public BaseTable {
    public:
        Table(uint32_t cardinality, uint32_t n_rows);

        void update(uint32_t rowid, int to_val);

        void remove(uint32_t rowid);

        void append(int val);

        int evaluate(uint32_t val, ibis::bitvector &res);

        int evaluate(uint32_t val, std::vector<uint64_t> &rids);

        void printMemory();

        void printUncompMemory();

    protected:
        uint32_t TBC;
        std::vector<uint32_t> append_rowids;
        ibis::bitvector existence_bitmap;

        void appendBitmap(int to_val);

        void fastAppend(int to_val);

        uint32_t total_rows;
    };

};

#endif
