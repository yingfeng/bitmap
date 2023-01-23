#ifndef TABLE_H_
#define TABLE_H_

#include <string>
#include <vector>
#include <thread>
#include <future>

#include "fastbit/bitvector.h"
#include "threadpool.h"

class BaseTable {
public:
    BaseTable(uint32_t cardinality, uint32_t numRows) : cardinality(cardinality), numRows(numRows), bitmaps(cardinality) {};

    virtual void update(unsigned int rowid, int to_val) = 0;

    virtual void remove(unsigned int rowid) = 0;

    virtual void append(int val) = 0;

    virtual int evaluate(uint32_t val, ibis::bitvector &res) = 0;

    virtual void printMemory() = 0;

    virtual void printUncompMemory() = 0;

    std::vector<ibis::bitvector *> bitmaps;

protected:
    const uint32_t cardinality;
    uint32_t numRows;

    std::string getBitmapName(int val) {
        std::stringstream ss;
        ss << kIndexPath << val << ".bm";
        return ss.str();
    }
};

namespace upbit {

    class Table : public BaseTable {
    public:
        Table(uint32_t cardinality, uint32_t numRows);

        void update(unsigned int rowid, int to_val);

        void remove(unsigned int rowid);

        void append(int val);

        int evaluate(uint32_t val, ibis::bitvector &res);

        int evaluate(uint32_t val, std::vector<uint64_t> &rids);

        void printMemory();

        void printUncompMemory();

        int getValue(uint32_t rowid);

        int range(uint32_t start, uint32_t range);

    protected:
        std::vector<ibis::bitvector *> updateBitmaps;
        uint32_t curtNumRows;
        ThreadPool *threadPool;

        inline int _getValue(uint32_t rowid, int curValue, bool *found);
    };

    class TableX {
    public:
        TableX(uint32_t cardinality);

        void appendRow(int val);

        void write(std::string dir);

    protected:
        const uint32_t cardinality;
        uint32_t numRows;

        std::vector<ibis::bitvector *> bitmaps;
    };

};

#endif
