#pragma once

// used generate compiler errors if native implementations are included directly
#ifndef _DTL_SIMD_INCLUDED
#define _DTL_SIMD_INCLUDED
#endif

#include "adept.hpp"
#include <x86intrin.h>

namespace dtl
{
namespace simd
{

struct bitwidth {
  static const u64
#ifdef __AVX512F__
  value = 512;
#elif __AVX2__
  value = 256;
#elif __SSE2__
  value = 128; // TODO reset to 128
#else
  value = 256; // emulated
#endif
};



template<typename T>
static constexpr u64 lane_count = bitwidth::value / (sizeof(T) * 8);

//  template<typename T>
//  struct lane_count {
//    static constexpr u64 value = bitwidth::value / (sizeof(T) * 8);
//  };


template<typename T>
struct lane {
  enum : u64 {
    count = bitwidth::value / (sizeof(T) * 8)
  };
};

} // namespace simd
} // namespace dtl


#include "simd/types.hpp"
//#include "simd/vec.hpp"
//#include "simd/extensions.hpp"


