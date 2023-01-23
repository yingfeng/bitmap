#include "upbit/util.h"
#include "fastbit/bitvector.h"

uint32_t kCardinality = 100;
std::string kDataPath;
std::string kIndexPath;
uint32_t kNumRows;
uint32_t kNumDeletes;
uint32_t kNumQueries;
uint32_t kMergeThreshold;
bool kVerbose;
bool kEnableFencePointer = false;
bool kShowMemoryUsage;
std::string kApproachName;
bool kShowBreakdown;
unsigned int kNumThreads = 0;
unsigned int kTimeOut;
std::string kRangeAlgoName = "pq";
unsigned int kQueryRange = 0;

double time_diff(struct timeval x, struct timeval y) {
    double x_ms , y_ms , diff;
    x_ms = (double) x.tv_sec * 1000000 + (double) x.tv_usec;
    y_ms = (double) y.tv_sec * 1000000 + (double) y.tv_usec;
    diff = y_ms - x_ms;
    return diff;
}

po::variables_map get_options(const int argc, const char *argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
            ("mode,m", po::value<std::string>(), "build / query / test")
            ("data-path,d", po::value<std::string>(), "data file path")
            ("index-path,i", po::value<std::string>(), "index file path")
            ("num-rows,n", po::value<uint32_t>(), "number of rows")
            ("num-queries", po::value<uint32_t>(), "number of queries")
            ("merge", po::value<uint32_t>()->default_value(10), "merge UB")
            ("verbose,v", po::value<bool>()->default_value(true), "verbose")
            ("removed,r", po::value<uint32_t>(), "number of deleted rows")
            ("show-memory", po::value<bool>()->default_value(false), "show memory")
            ("help", "produce help message")
            ("approach,a", po::value<std::string>()->default_value(std::string("ub")), "in-place, ucb, ub")
            ("fence-length", po::value<unsigned int>()->default_value(1000), "lengh of fence pointers")
            ("breakdown", po::value<bool>()->default_value(false), "breakdown")
            ("num-threads", po::value<unsigned int>()->default_value(4), "num of threads")
            ("time-out,t", po::value<unsigned int>()->default_value(0), "time out (s)")
            ("cardinality,c", boost::program_options::value<uint32_t>()->default_value(1), "cardinality");

    po::positional_options_description p;
    p.add("mode", -1);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }

    return vm;
}

void parse(po::variables_map options) {
    if (options.count("data-path"))
        kDataPath = options["data-path"].as<std::string>();
    if (options.count("cardinality"))
        kCardinality = options["cardinality"].as<uint32_t>();
    if (options.count("index-path"))
        kIndexPath = options["index-path"].as<std::string>();
    if (options.count("num-rows"))
        kNumRows = options["num-rows"].as<uint32_t>();
    if (options.count("removed"))
        kNumDeletes = options["removed"].as<uint32_t>();
    if (options.count("num-queries"))
        kNumQueries = options["num-queries"].as<uint32_t>();
    kMergeThreshold = options["merge"].as<uint32_t>();
    kVerbose = options["verbose"].as<bool>();
    kShowMemoryUsage = options["show-memory"].as<bool>();
    kApproachName = options["approach"].as<std::string>();
    INDEX_WORDS = options["fence-length"].as<unsigned int>();
    kShowBreakdown = options["breakdown"].as<bool>();
    kNumThreads = options["num-threads"].as<unsigned int>();
    kTimeOut = options["time-out"].as<unsigned int>();

    if (kApproachName == "ub") {
        kEnableFencePointer = true;
    }
}
