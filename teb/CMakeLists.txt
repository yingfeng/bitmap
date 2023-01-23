cmake_minimum_required(VERSION 3.5)
project(navin)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -march=native -Wno-sign-compare")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -march=native -mavx512bw -mavx512f")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -march=core-avx2")

# HACK: This gives access to the private members of the boost::dynamic_bitset.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS")

# Enable ASAN in debug builds.
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

include_directories(src)


# Roaring Bitmaps (deprecated since we switched to the unity build)
#add_subdirectory(../CRoaring CRoaring)
#include_directories(../CRoaring/include)


include_directories(SYSTEM /usr/local/include)

include_directories(../) #TODO remove

include_directories(./)

add_subdirectory(thirdparty/gtest)
include_directories(thirdparty/gtest/include lib/googletest)
include_directories(thirdparty)

##===----------------------------------------------------------------------===##


# The following flag disables the inlining of TEB function.
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEB_NO_INLINE")

set(TEB_SOURCE_FILES
        src/dtl/bitmap.hpp
        src/dtl/bitmap/diff/diff.hpp
        src/dtl/bitmap/diff/merge.hpp
        src/dtl/bitmap/diff/merge_teb.hpp
        src/dtl/bitmap/part/part.hpp
        src/dtl/bitmap/part/part_run.hpp
        src/dtl/bitmap/part/part_updirect.hpp
        src/dtl/bitmap/part/part_upforward.hpp
        src/dtl/bitmap/util/binary_tree_structure.hpp
        src/dtl/bitmap/util/bit_buffer.hpp
        src/dtl/bitmap/util/bit_buffer_avx2.hpp
        src/dtl/bitmap/util/bit_buffer_avx512.hpp
        src/dtl/bitmap/util/bitmap_fun.hpp
        src/dtl/bitmap/util/bitmap_seq_reader.hpp
        src/dtl/bitmap/util/bitmap_tree.hpp
        src/dtl/bitmap/util/bitmap_view.hpp
        src/dtl/bitmap/util/bitmap_writer.hpp
        src/dtl/bitmap/util/buffer.hpp
        src/dtl/bitmap/util/mutable_bitmap_tree.hpp
        src/dtl/bitmap/util/plain_bitmap.hpp
        src/dtl/bitmap/util/plain_bitmap_iter.hpp
        src/dtl/bitmap/util/rank1_logic_linear.hpp
        src/dtl/bitmap/util/rank1.hpp
        src/dtl/bitmap/util/rank1_logic_surf.hpp
        src/dtl/bitmap/util/rank1_logic_word_blocked.hpp
        src/dtl/bitmap/bitwise_operations.hpp
        src/dtl/bitmap/iterator.hpp
        src/dtl/bitmap/teb_builder.hpp
        src/dtl/bitmap/teb_flat.hpp
        src/dtl/bitmap/teb_iter.hpp
        src/dtl/bitmap/teb_wrapper.hpp
        src/dtl/bitmap/teb_scan_iter.hpp
        src/dtl/bitmap/teb_scan_util.hpp
        src/dtl/bitmap/teb_types.hpp
        src/dtl/iterator.hpp
        src/dtl/static_stack.hpp
        src/dtl/static_stack2.hpp
        )

set(SOURCE_FILES
        ${TEB_SOURCE_FILES}
        )


##===----------------------------------------------------------------------===##
include(CMakeListsLocal.cmake OPTIONAL)
##===----------------------------------------------------------------------===##

##===----------------------------------------------------------------------===##
# Test cases.
##===----------------------------------------------------------------------===##
set(TEST_FILES
        ${SOURCE_FILES}
        test/dtl/bitmap/util/bit_buffer_avx2_test.cpp
        test/dtl/bitmap/util/bit_buffer_avx512_test.cpp
        test/dtl/bitmap/util/bit_buffer_test.cpp
        test/dtl/bitmap/util/bitmap_fun_test.cpp
        #test/dtl/bitmap/util/rank_test.cpp
        test/dtl/bitmap/api_types.hpp
        #test/dtl/bitmap/api_encode_decode_test.cpp
        #test/dtl/bitmap/api_random_access_test.cpp
        #test/dtl/bitmap/api_run_iterator_test.cpp
        #test/dtl/bitmap/api_run_iterator_skip_test.cpp
        #test/dtl/bitmap/api_bitwise_operation_test.cpp
        #test/dtl/bitmap/bitwise_operations_helper.hpp
        #test/dtl/bitmap/diff_test.cpp
        #test/dtl/bitmap/part_diff_test.cpp
        #test/dtl/bitmap/plain_bitmap_iter_test.cpp
        #test/dtl/bitmap/update_test.cpp
        test/dtl/bitmap/teb_scan_util_test.cpp
        )
add_executable(tester ${TEST_FILES})
target_link_libraries(tester gtest gtest_main dl)

set(MY_TEST
        ${TEB_SOURCE_FILES}
        src/dtl/bitmap/my_test.cpp
        )
add_executable(my_test ${MY_TEST})

set(UPDATE_TEST
        ${TEB_SOURCE_FILES}
        experiments/util/prune_util.hpp
        experiments/util/update_test.cpp
        )
add_executable(update_test ${UPDATE_TEST})
