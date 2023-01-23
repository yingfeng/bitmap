cmake_minimum_required(VERSION 3.5)
project(BSI)

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


add_subdirectory(CRoaring)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/CRoaring/include)

include_directories(SYSTEM /usr/local/include)

include_directories(./)


# The following flag disables the inlining of TEB function.
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEB_NO_INLINE")

set(SOURCE_FILES
        "common.cpp"
        "cache.cpp"
        "bsi.cpp"
        )

add_library(bsi ${SOURCE_FILES})

#add_executable(tester ${SOURCE_FILES})
target_link_libraries(bsi roaring dl)