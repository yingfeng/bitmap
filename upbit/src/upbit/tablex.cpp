#include "fastbit/bitvector.h"

#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <sys/mman.h>
#include <queue>	// priority queue

#include "table.h"
#include "util.h"

using namespace upbit;

TableX::TableX(uint32_t cardinality) : cardinality(cardinality), numRows(0) {
    for (int i = 0; i < cardinality; ++i) {
        bitmaps.push_back(new ibis::bitvector());
    }
}

void TableX::appendRow(int val) {
    for (int i = 0; i < cardinality; ++i) {
        bitmaps[i]->setBit(numRows, i == val);
    }
    numRows += 1;
}

void TableX::write(std::string dir) {
    boost::filesystem::path dir_path(dir);
    boost::filesystem::create_directory(dir_path);
    for (int i = 0; i < cardinality; ++i) {
        std::stringstream ss;
        ss << dir << i << ".bm";
        bitmaps[i]->adjustSize(0, numRows);
        bitmaps[i]->write(ss.str().c_str());
    }
}

