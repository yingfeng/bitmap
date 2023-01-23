#include <iostream>
#include <sys/mman.h>
#include <vector>
#include <random>
#include <chrono>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "fastbit/bitvector.h"
#include "upbit/table.h"
#include "upbit/util.h"
#include "ucb/table.h"
#include "inplace/table.h"

using namespace std;
using namespace upbit;

void buildIndex() {
    ibis::horometer timer;
    timer.start();

    int *src;
    int fdes = open(kDataPath.c_str(), OPEN_READONLY);
    struct stat statbuf;
    fstat(fdes, &statbuf);
    src = (int *) mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fdes, 0);

    TableX table(kCardinality);
    for (int i = 0; i < kNumRows; ++i) {
        table.appendRow(src[i] - 1);
    }
    table.write(kIndexPath);

    timer.stop();
    std::cout << "upbit:: build_index() took "
              << timer.CPUTime() << " CPU seconds, "
              << timer.realTime() << " elapsed seconds" << endl;
}

template<typename T>
void evaluate(string mode) {
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    uniform_int_distribution<int> distribution(0, kCardinality - 1);
    uniform_int_distribution<uint32_t> rid_distribution(0, kNumRows - 1);

    T table(kCardinality, kNumRows);

    struct timeval before, after;
    struct timeval time_out_begin, time_out_end;
    vector<double> times;

    double ratio = (double) kNumDeletes / (double) (kNumDeletes + kNumQueries);
    cout << "ratio = " << ratio << endl;

    uniform_real_distribution<double> ratio_distribution(0.0, 1.0);
    string _mode = mode;

    if (kShowMemoryUsage) {
        table.printMemory();
        table.printUncompMemory();
    }

    vector<uint64_t> rids(kNumRows / 10);

    gettimeofday(&time_out_begin, NULL);
    for (int i = 0; i < kNumQueries + kNumDeletes; ++i) {
        if (ratio_distribution(generator) < ratio) {
            uint32_t rid = rid_distribution(generator);
            int val = distribution(generator);
            if (_mode == "mix") {
                int test = rid_distribution(generator);
                switch (test % 3) {
                    case 0:
                        mode = "update";
                        break;
                    case 1:
                        mode = "delete";
                        break;
                    case 2:
                        mode = "insert";
                        break;
                }
            }
            if (mode == "update") {
                gettimeofday(&before, NULL);
                table.update(rid, val);
                gettimeofday(&after, NULL);
                times.push_back(time_diff(before, after));
                if (kVerbose) {
                    cout << "U " << time_diff(before, after) << endl;
                }
            }
            else if (mode == "delete") {
                gettimeofday(&before, NULL);
                table.remove(rid);
                gettimeofday(&after, NULL);
                times.push_back(time_diff(before, after));
                if (kVerbose)
                    cout << "D " << time_diff(before, after) << endl;
            }
            else if (mode == "insert") {
                gettimeofday(&before, NULL);
                table.append(val);
                gettimeofday(&after, NULL);
                times.push_back(time_diff(before, after));
                if (kVerbose)
                    cout << "I " << time_diff(before, after) << endl;
            }
        } else {
            int r = distribution(generator);
            // ibis::bitvector res;
            gettimeofday(&before, NULL);
            table.evaluate(r, rids);
            gettimeofday(&after, NULL);
            times.push_back(time_diff(before, after));
            if (kVerbose)
                cout << "Q " << time_diff(before, after) << endl;
            rids.clear();
        }

        if (kShowMemoryUsage && i % 1000 == 0) {
            table.printMemory();
            table.printUncompMemory();
        }

        if (times.size() > 1000 && !kVerbose) {
            double sum = std::accumulate(times.begin(), times.end(), 0.0);
            double mean = sum / times.size();
            double sq_sum = std::inner_product(times.begin(), times.end(), times.begin(), 0.0);
            double stddev = std::sqrt(sq_sum / times.size() - mean * mean);
            printf("time:\t %.0lf us \tstddev: %.0lf us\n", mean, stddev);
            times.clear();
        }

        if (kTimeOut > 0 && i % 10 == 0) {
            gettimeofday(&time_out_end, NULL);
            if (time_diff(time_out_begin, time_out_end) > kTimeOut * 1000000) {
                break;
            }
        }
    }

    if (kShowMemoryUsage) {
        table.printMemory();
        table.printUncompMemory();
    }
}

//void evaluateGetValue();
//void evaluateImpact() {
//    auto seed = chrono::system_clock::now().time_since_epoch().count();
//    default_random_engine generator(seed);
//    uniform_int_distribution<int> distribution(0, kCardinality - 1);
//    uniform_int_distribution<uint32_t> rid_distribution(0, kNumRows - 1);
//
//    struct timeval before, after;
//    struct timeval time_out_begin, time_out_end;
//
//    uniform_real_distribution<double> ratio_distribution(0.0, 1.0);
//
//    int comp[] = {5,10,15,20,50,100,200,300,500,1000,2000,5000,10000};
//
//    gettimeofday(&time_out_begin, NULL);
//    for (int i = 0; i < 13; ++i) {
//        BaseTable *table = new upbit::Table(kCardinality, kNumRows);
//        evaluateGetValue();
//
//        int thres = comp[i];
//
//        cout << "Current comp: " << thres << endl;
//
//        for (int j = 0; j < thres; ++j) {
//            uint32_t rid = rid_distribution(generator);
//            int val = 0;
//            gettimeofday(&before, NULL);
//            table->update(rid, val);
//            gettimeofday(&after, NULL);
//            cout << "U " << time_diff(before, after) << endl;
//        }
//
//        ibis::bitvector res;
//        gettimeofday(&before, NULL);
//        table->evaluate(0, res);
//        gettimeofday(&after, NULL);
//        cout << "Q " << time_diff(before, after) << endl;
//    }
//}
//
//void evaluateGetValue() {
//    upbit::Table table(kCardinality, kNumRows);
//
//    auto seed = chrono::system_clock::now().time_since_epoch().count();
//    default_random_engine generator(seed);
//    uniform_int_distribution<uint32_t> distribution(0, kNumRows - 1);
//
//    struct timeval before, after;
//    gettimeofday(&before, NULL);
//    for (int i = 0; i < 10000; ++i) {
//        int test = distribution(generator);
//        table.getValue(test);
//    }
//    gettimeofday(&after, NULL);
//    cout << time_diff(before, after) << endl;
//}
//
//void evaluateRange() {
//    auto seed = chrono::system_clock::now().time_since_epoch().count();
//    default_random_engine generator(seed);
//    uniform_int_distribution<uint32_t> distribution(0, kCardinality - 1 - kQueryRange);
//
//    upbit::Table *table = new upbit::Table(kCardinality, kNumRows);
//
//    struct timeval before, after;
//    struct timeval time_out_begin, time_out_end;
//
//    cout << "range = " << kQueryRange << endl;
//
//    gettimeofday(&time_out_begin, NULL);
//    for (int i = 0; i < kQueryRange; ++i) {
//        auto r = distribution(generator);
//        gettimeofday(&before, NULL);
//        table->range(r, kQueryRange);
//        gettimeofday(&after, NULL);
//
//        if (kVerbose)
//            cout << "Q " << time_diff(before, after) << endl;
//
//        if (kTimeOut > 0 && i % 10 == 0) {
//            gettimeofday(&time_out_end, NULL);
//            if (time_diff(time_out_begin, time_out_end) > kTimeOut * 1000000) {
//                break;
//            }
//        }
//    }
//}

int main(const int argc, const char *argv[]) {
    po::variables_map options = get_options(argc, argv);
    parse(options);

    if (options.count("mode")) {
        string mode = options["mode"].as<string>();
        if (mode == "build") {
            buildIndex();
        } else if (mode == "getvalue") {
            // evaluateGetValue();
        } else if (mode == "impact") {
            // evaluateImpact();
        } else if (mode == "range") {
            // evaluateRange();
        } else {
            if (kApproachName == "ub") {
                evaluate<upbit::Table>(mode);
            } else if (kApproachName == "ucb") {
                evaluate<ucb::Table>(mode);
            } else if (kApproachName == "inplace") {
                evaluate<inplace::Table>(mode);
            } else {
                cerr << "Unknown approach name." << endl;
                exit(-1);
            }
        }
    }

    return 0;
}