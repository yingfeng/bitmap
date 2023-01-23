#ifndef NAIVE_TABLE_H_
#define NAIVE_TABLE_H_

#include "fastbit/bitvector.h"
#include "upbit/table.h"

#include <string>
#include <vector>

namespace inplace {
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

        int get_value(uint32_t rowid);

    protected:
        ThreadPool *threadPool;

        int _get_value(uint32_t rowid, int curValue, volatile bool &flag);
    };
};

#endif
