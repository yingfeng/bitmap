#include <string>
#include <iostream>
#include <numeric> // std::accumulate
#include <cmath>   // std::minus
#include <boost/program_options.hpp>

#ifndef NICOLAS_UTIL_H
#define NICOLAS_UTIL_H

namespace po = boost::program_options;

double time_diff(struct timeval x, struct timeval y);

po::variables_map get_options(const int argc, const char *argv[]);

void parse(po::variables_map options);

extern uint32_t kCardinality;
extern std::string kDataPath;
extern std::string kIndexPath;
extern uint32_t kNumRows;
extern uint32_t kNumDeletes;
extern uint32_t kNumQueries;
extern uint32_t kMergeThreshold;
extern bool kVerbose;
extern bool kEnableFencePointer;
extern bool kShowMemoryUsage;
extern std::string kApproachName;
extern bool kShowBreakdown;
extern unsigned int kNumThreads;
extern unsigned int kTimeOut;
extern std::string kRangeAlgoName;
extern unsigned int kQueryRange;

template<typename T>
inline T volatile &access_once(T &t) {
    return static_cast<T volatile &>(t);
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#endif
