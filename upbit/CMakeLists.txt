cmake_minimum_required(VERSION 2.8.4)

option(test "Build all tests." OFF) # Makes boolean 'test' available.

set(PROJECT_NAME upbit)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++11 -msse4.2")

include_directories(${CMAKE_SOURCE_DIR}/src)

# boost
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # OS X
    set(FASTBIF_LIBRARY "/usr/local/lib/libfastbit.0.dylib")
    set(CMAKE_CXX_FLAGS  "-std=c++11 -stdlib=libc++ -msse4.2")

    find_package(Boost COMPONENTS filesystem program_options system random REQUIRED)
    IF (Boost_FOUND)
        INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
        ADD_DEFINITIONS( "-DHAS_BOOST" )
    ENDIF()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux
    set(FASTBIF_LIBRARY "~/usr/local/lib/libfastbit.so")
    include_directories("~/usr/local/include")
    set(CMAKE_CXX_FLAGS  "-std=c++11 -msse4.2")

    set(Boost_LIBRARIES "~/usr/local/lib/libboost_system.so;~/usr/local/lib/libboost_filesystem.so;~/usr/local/lib/libboost_program_options.so")

    ADD_DEFINITIONS( "-DLINUX" )

    # openmp
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# pthread
find_package (Threads)

# fastbit
set(FASTBIT_FILES src/fastbit/bitvector.cpp src/fastbit/bitvector.h
                  src/fastbit/fileManager.cpp src/fastbit/fileManager.h
                  src/fastbit/resource.cpp src/fastbit/resource.h
                  src/fastbit/util.cpp src/fastbit/util.h
                  src/fastbit/array_t.cpp src/fastbit/array_t.h)

# upbit
set(UB_SOURCE_FILES src/upbit/table.cpp src/upbit/tablex.cpp src/upbit/table.h src/upbit/util.h src/upbit/util.cpp)
set(UCB_SOURCE_FILES src/ucb/table.h src/ucb/table.cpp)
set(NAIVE_SOURCE_FILES src/inplace/table.h src/inplace/table.cpp)

add_executable(upbit src/main.cpp ${UB_SOURCE_FILES} ${UCB_SOURCE_FILES} ${NAIVE_SOURCE_FILES} ${FASTBIT_FILES} src/upbit/perf.h src/threadpool.h)
target_link_libraries(upbit ${Boost_LIBRARIES})
