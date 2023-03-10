#pragma once

#ifndef _DTL_SIMD_INCLUDED
#error "Never use <dtl/simd/types.hpp> directly; include <dtl/simd.hpp> instead."
#endif

#include <dtl/dtl.hpp>

#include "emmintrin.h"
#include "smmintrin.h"

#if defined(__AVX2__)
#include "immintrin.h"
#endif


namespace dtl
{

union alignas(16)
  r128 {
  $u64 u64[2];
  $i64 i64[2];

  $u32 u32[4];
  $i32 i32[4];

  $u16 u16[8];
  $i16 i16[8];

  $u8 u8[16];
  $i8 i8[16];

  __m128i i; // integer data
  __m128  s; // single precision floating point data
  __m128d d; // double precision floating point data
};

#if defined(__AVX2__)
union alignas(32)
  r256 {
  $u64 u64[4];
  $i64 i64[4];

  $u32 u32[8];
  $i32 i32[8];

  $u16 u16[16];
  $i16 i16[16];

  $u8 u8[32];
  $i8 i8[32];

  __m256i i; // integer data
  __m256  s; // single precision floating point data
  __m256d d; // double precision floating point data

  dtl::r128 r128[2];
};


namespace simd
{

/// Lookup table used to convert an 8-bit mask into a 32-bit position list
constexpr dtl::r128 lut_match_pos[256] = {
  { .i16 = { -1, -1, -1, -1, -1, -1, -1, -1 } }, // 0
  { .i16 = { 0, -1, -1, -1, -1, -1, -1, -1 } }, // 1
  { .i16 = { 1, -1, -1, -1, -1, -1, -1, -1 } }, // 2
  { .i16 = { 0, 1, -1, -1, -1, -1, -1, -1 } }, // 3
  { .i16 = { 2, -1, -1, -1, -1, -1, -1, -1 } }, // 4
  { .i16 = { 0, 2, -1, -1, -1, -1, -1, -1 } }, // 5
  { .i16 = { 1, 2, -1, -1, -1, -1, -1, -1 } }, // 6
  { .i16 = { 0, 1, 2, -1, -1, -1, -1, -1 } }, // 7
  { .i16 = { 3, -1, -1, -1, -1, -1, -1, -1 } }, // 8
  { .i16 = { 0, 3, -1, -1, -1, -1, -1, -1 } }, // 9
  { .i16 = { 1, 3, -1, -1, -1, -1, -1, -1 } }, // 10
  { .i16 = { 0, 1, 3, -1, -1, -1, -1, -1 } }, // 11
  { .i16 = { 2, 3, -1, -1, -1, -1, -1, -1 } }, // 12
  { .i16 = { 0, 2, 3, -1, -1, -1, -1, -1 } }, // 13
  { .i16 = { 1, 2, 3, -1, -1, -1, -1, -1 } }, // 14
  { .i16 = { 0, 1, 2, 3, -1, -1, -1, -1 } }, // 15
  { .i16 = { 4, -1, -1, -1, -1, -1, -1, -1 } }, // 16
  { .i16 = { 0, 4, -1, -1, -1, -1, -1, -1 } }, // 17
  { .i16 = { 1, 4, -1, -1, -1, -1, -1, -1 } }, // 18
  { .i16 = { 0, 1, 4, -1, -1, -1, -1, -1 } }, // 19
  { .i16 = { 2, 4, -1, -1, -1, -1, -1, -1 } }, // 20
  { .i16 = { 0, 2, 4, -1, -1, -1, -1, -1 } }, // 21
  { .i16 = { 1, 2, 4, -1, -1, -1, -1, -1 } }, // 22
  { .i16 = { 0, 1, 2, 4, -1, -1, -1, -1 } }, // 23
  { .i16 = { 3, 4, -1, -1, -1, -1, -1, -1 } }, // 24
  { .i16 = { 0, 3, 4, -1, -1, -1, -1, -1 } }, // 25
  { .i16 = { 1, 3, 4, -1, -1, -1, -1, -1 } }, // 26
  { .i16 = { 0, 1, 3, 4, -1, -1, -1, -1 } }, // 27
  { .i16 = { 2, 3, 4, -1, -1, -1, -1, -1 } }, // 28
  { .i16 = { 0, 2, 3, 4, -1, -1, -1, -1 } }, // 29
  { .i16 = { 1, 2, 3, 4, -1, -1, -1, -1 } }, // 30
  { .i16 = { 0, 1, 2, 3, 4, -1, -1, -1 } }, // 31
  { .i16 = { 5, -1, -1, -1, -1, -1, -1, -1 } }, // 32
  { .i16 = { 0, 5, -1, -1, -1, -1, -1, -1 } }, // 33
  { .i16 = { 1, 5, -1, -1, -1, -1, -1, -1 } }, // 34
  { .i16 = { 0, 1, 5, -1, -1, -1, -1, -1 } }, // 35
  { .i16 = { 2, 5, -1, -1, -1, -1, -1, -1 } }, // 36
  { .i16 = { 0, 2, 5, -1, -1, -1, -1, -1 } }, // 37
  { .i16 = { 1, 2, 5, -1, -1, -1, -1, -1 } }, // 38
  { .i16 = { 0, 1, 2, 5, -1, -1, -1, -1 } }, // 39
  { .i16 = { 3, 5, -1, -1, -1, -1, -1, -1 } }, // 40
  { .i16 = { 0, 3, 5, -1, -1, -1, -1, -1 } }, // 41
  { .i16 = { 1, 3, 5, -1, -1, -1, -1, -1 } }, // 42
  { .i16 = { 0, 1, 3, 5, -1, -1, -1, -1 } }, // 43
  { .i16 = { 2, 3, 5, -1, -1, -1, -1, -1 } }, // 44
  { .i16 = { 0, 2, 3, 5, -1, -1, -1, -1 } }, // 45
  { .i16 = { 1, 2, 3, 5, -1, -1, -1, -1 } }, // 46
  { .i16 = { 0, 1, 2, 3, 5, -1, -1, -1 } }, // 47
  { .i16 = { 4, 5, -1, -1, -1, -1, -1, -1 } }, // 48
  { .i16 = { 0, 4, 5, -1, -1, -1, -1, -1 } }, // 49
  { .i16 = { 1, 4, 5, -1, -1, -1, -1, -1 } }, // 50
  { .i16 = { 0, 1, 4, 5, -1, -1, -1, -1 } }, // 51
  { .i16 = { 2, 4, 5, -1, -1, -1, -1, -1 } }, // 52
  { .i16 = { 0, 2, 4, 5, -1, -1, -1, -1 } }, // 53
  { .i16 = { 1, 2, 4, 5, -1, -1, -1, -1 } }, // 54
  { .i16 = { 0, 1, 2, 4, 5, -1, -1, -1 } }, // 55
  { .i16 = { 3, 4, 5, -1, -1, -1, -1, -1 } }, // 56
  { .i16 = { 0, 3, 4, 5, -1, -1, -1, -1 } }, // 57
  { .i16 = { 1, 3, 4, 5, -1, -1, -1, -1 } }, // 58
  { .i16 = { 0, 1, 3, 4, 5, -1, -1, -1 } }, // 59
  { .i16 = { 2, 3, 4, 5, -1, -1, -1, -1 } }, // 60
  { .i16 = { 0, 2, 3, 4, 5, -1, -1, -1 } }, // 61
  { .i16 = { 1, 2, 3, 4, 5, -1, -1, -1 } }, // 62
  { .i16 = { 0, 1, 2, 3, 4, 5, -1, -1 } }, // 63
  { .i16 = { 6, -1, -1, -1, -1, -1, -1, -1 } }, // 64
  { .i16 = { 0, 6, -1, -1, -1, -1, -1, -1 } }, // 65
  { .i16 = { 1, 6, -1, -1, -1, -1, -1, -1 } }, // 66
  { .i16 = { 0, 1, 6, -1, -1, -1, -1, -1 } }, // 67
  { .i16 = { 2, 6, -1, -1, -1, -1, -1, -1 } }, // 68
  { .i16 = { 0, 2, 6, -1, -1, -1, -1, -1 } }, // 69
  { .i16 = { 1, 2, 6, -1, -1, -1, -1, -1 } }, // 70
  { .i16 = { 0, 1, 2, 6, -1, -1, -1, -1 } }, // 71
  { .i16 = { 3, 6, -1, -1, -1, -1, -1, -1 } }, // 72
  { .i16 = { 0, 3, 6, -1, -1, -1, -1, -1 } }, // 73
  { .i16 = { 1, 3, 6, -1, -1, -1, -1, -1 } }, // 74
  { .i16 = { 0, 1, 3, 6, -1, -1, -1, -1 } }, // 75
  { .i16 = { 2, 3, 6, -1, -1, -1, -1, -1 } }, // 76
  { .i16 = { 0, 2, 3, 6, -1, -1, -1, -1 } }, // 77
  { .i16 = { 1, 2, 3, 6, -1, -1, -1, -1 } }, // 78
  { .i16 = { 0, 1, 2, 3, 6, -1, -1, -1 } }, // 79
  { .i16 = { 4, 6, -1, -1, -1, -1, -1, -1 } }, // 80
  { .i16 = { 0, 4, 6, -1, -1, -1, -1, -1 } }, // 81
  { .i16 = { 1, 4, 6, -1, -1, -1, -1, -1 } }, // 82
  { .i16 = { 0, 1, 4, 6, -1, -1, -1, -1 } }, // 83
  { .i16 = { 2, 4, 6, -1, -1, -1, -1, -1 } }, // 84
  { .i16 = { 0, 2, 4, 6, -1, -1, -1, -1 } }, // 85
  { .i16 = { 1, 2, 4, 6, -1, -1, -1, -1 } }, // 86
  { .i16 = { 0, 1, 2, 4, 6, -1, -1, -1 } }, // 87
  { .i16 = { 3, 4, 6, -1, -1, -1, -1, -1 } }, // 88
  { .i16 = { 0, 3, 4, 6, -1, -1, -1, -1 } }, // 89
  { .i16 = { 1, 3, 4, 6, -1, -1, -1, -1 } }, // 90
  { .i16 = { 0, 1, 3, 4, 6, -1, -1, -1 } }, // 91
  { .i16 = { 2, 3, 4, 6, -1, -1, -1, -1 } }, // 92
  { .i16 = { 0, 2, 3, 4, 6, -1, -1, -1 } }, // 93
  { .i16 = { 1, 2, 3, 4, 6, -1, -1, -1 } }, // 94
  { .i16 = { 0, 1, 2, 3, 4, 6, -1, -1 } }, // 95
  { .i16 = { 5, 6, -1, -1, -1, -1, -1, -1 } }, // 96
  { .i16 = { 0, 5, 6, -1, -1, -1, -1, -1 } }, // 97
  { .i16 = { 1, 5, 6, -1, -1, -1, -1, -1 } }, // 98
  { .i16 = { 0, 1, 5, 6, -1, -1, -1, -1 } }, // 99
  { .i16 = { 2, 5, 6, -1, -1, -1, -1, -1 } }, // 100
  { .i16 = { 0, 2, 5, 6, -1, -1, -1, -1 } }, // 101
  { .i16 = { 1, 2, 5, 6, -1, -1, -1, -1 } }, // 102
  { .i16 = { 0, 1, 2, 5, 6, -1, -1, -1 } }, // 103
  { .i16 = { 3, 5, 6, -1, -1, -1, -1, -1 } }, // 104
  { .i16 = { 0, 3, 5, 6, -1, -1, -1, -1 } }, // 105
  { .i16 = { 1, 3, 5, 6, -1, -1, -1, -1 } }, // 106
  { .i16 = { 0, 1, 3, 5, 6, -1, -1, -1 } }, // 107
  { .i16 = { 2, 3, 5, 6, -1, -1, -1, -1 } }, // 108
  { .i16 = { 0, 2, 3, 5, 6, -1, -1, -1 } }, // 109
  { .i16 = { 1, 2, 3, 5, 6, -1, -1, -1 } }, // 110
  { .i16 = { 0, 1, 2, 3, 5, 6, -1, -1 } }, // 111
  { .i16 = { 4, 5, 6, -1, -1, -1, -1, -1 } }, // 112
  { .i16 = { 0, 4, 5, 6, -1, -1, -1, -1 } }, // 113
  { .i16 = { 1, 4, 5, 6, -1, -1, -1, -1 } }, // 114
  { .i16 = { 0, 1, 4, 5, 6, -1, -1, -1 } }, // 115
  { .i16 = { 2, 4, 5, 6, -1, -1, -1, -1 } }, // 116
  { .i16 = { 0, 2, 4, 5, 6, -1, -1, -1 } }, // 117
  { .i16 = { 1, 2, 4, 5, 6, -1, -1, -1 } }, // 118
  { .i16 = { 0, 1, 2, 4, 5, 6, -1, -1 } }, // 119
  { .i16 = { 3, 4, 5, 6, -1, -1, -1, -1 } }, // 120
  { .i16 = { 0, 3, 4, 5, 6, -1, -1, -1 } }, // 121
  { .i16 = { 1, 3, 4, 5, 6, -1, -1, -1 } }, // 122
  { .i16 = { 0, 1, 3, 4, 5, 6, -1, -1 } }, // 123
  { .i16 = { 2, 3, 4, 5, 6, -1, -1, -1 } }, // 124
  { .i16 = { 0, 2, 3, 4, 5, 6, -1, -1 } }, // 125
  { .i16 = { 1, 2, 3, 4, 5, 6, -1, -1 } }, // 126
  { .i16 = { 0, 1, 2, 3, 4, 5, 6, -1 } }, // 127
  { .i16 = { 7, -1, -1, -1, -1, -1, -1, -1 } }, // 128
  { .i16 = { 0, 7, -1, -1, -1, -1, -1, -1 } }, // 129
  { .i16 = { 1, 7, -1, -1, -1, -1, -1, -1 } }, // 130
  { .i16 = { 0, 1, 7, -1, -1, -1, -1, -1 } }, // 131
  { .i16 = { 2, 7, -1, -1, -1, -1, -1, -1 } }, // 132
  { .i16 = { 0, 2, 7, -1, -1, -1, -1, -1 } }, // 133
  { .i16 = { 1, 2, 7, -1, -1, -1, -1, -1 } }, // 134
  { .i16 = { 0, 1, 2, 7, -1, -1, -1, -1 } }, // 135
  { .i16 = { 3, 7, -1, -1, -1, -1, -1, -1 } }, // 136
  { .i16 = { 0, 3, 7, -1, -1, -1, -1, -1 } }, // 137
  { .i16 = { 1, 3, 7, -1, -1, -1, -1, -1 } }, // 138
  { .i16 = { 0, 1, 3, 7, -1, -1, -1, -1 } }, // 139
  { .i16 = { 2, 3, 7, -1, -1, -1, -1, -1 } }, // 140
  { .i16 = { 0, 2, 3, 7, -1, -1, -1, -1 } }, // 141
  { .i16 = { 1, 2, 3, 7, -1, -1, -1, -1 } }, // 142
  { .i16 = { 0, 1, 2, 3, 7, -1, -1, -1 } }, // 143
  { .i16 = { 4, 7, -1, -1, -1, -1, -1, -1 } }, // 144
  { .i16 = { 0, 4, 7, -1, -1, -1, -1, -1 } }, // 145
  { .i16 = { 1, 4, 7, -1, -1, -1, -1, -1 } }, // 146
  { .i16 = { 0, 1, 4, 7, -1, -1, -1, -1 } }, // 147
  { .i16 = { 2, 4, 7, -1, -1, -1, -1, -1 } }, // 148
  { .i16 = { 0, 2, 4, 7, -1, -1, -1, -1 } }, // 149
  { .i16 = { 1, 2, 4, 7, -1, -1, -1, -1 } }, // 150
  { .i16 = { 0, 1, 2, 4, 7, -1, -1, -1 } }, // 151
  { .i16 = { 3, 4, 7, -1, -1, -1, -1, -1 } }, // 152
  { .i16 = { 0, 3, 4, 7, -1, -1, -1, -1 } }, // 153
  { .i16 = { 1, 3, 4, 7, -1, -1, -1, -1 } }, // 154
  { .i16 = { 0, 1, 3, 4, 7, -1, -1, -1 } }, // 155
  { .i16 = { 2, 3, 4, 7, -1, -1, -1, -1 } }, // 156
  { .i16 = { 0, 2, 3, 4, 7, -1, -1, -1 } }, // 157
  { .i16 = { 1, 2, 3, 4, 7, -1, -1, -1 } }, // 158
  { .i16 = { 0, 1, 2, 3, 4, 7, -1, -1 } }, // 159
  { .i16 = { 5, 7, -1, -1, -1, -1, -1, -1 } }, // 160
  { .i16 = { 0, 5, 7, -1, -1, -1, -1, -1 } }, // 161
  { .i16 = { 1, 5, 7, -1, -1, -1, -1, -1 } }, // 162
  { .i16 = { 0, 1, 5, 7, -1, -1, -1, -1 } }, // 163
  { .i16 = { 2, 5, 7, -1, -1, -1, -1, -1 } }, // 164
  { .i16 = { 0, 2, 5, 7, -1, -1, -1, -1 } }, // 165
  { .i16 = { 1, 2, 5, 7, -1, -1, -1, -1 } }, // 166
  { .i16 = { 0, 1, 2, 5, 7, -1, -1, -1 } }, // 167
  { .i16 = { 3, 5, 7, -1, -1, -1, -1, -1 } }, // 168
  { .i16 = { 0, 3, 5, 7, -1, -1, -1, -1 } }, // 169
  { .i16 = { 1, 3, 5, 7, -1, -1, -1, -1 } }, // 170
  { .i16 = { 0, 1, 3, 5, 7, -1, -1, -1 } }, // 171
  { .i16 = { 2, 3, 5, 7, -1, -1, -1, -1 } }, // 172
  { .i16 = { 0, 2, 3, 5, 7, -1, -1, -1 } }, // 173
  { .i16 = { 1, 2, 3, 5, 7, -1, -1, -1 } }, // 174
  { .i16 = { 0, 1, 2, 3, 5, 7, -1, -1 } }, // 175
  { .i16 = { 4, 5, 7, -1, -1, -1, -1, -1 } }, // 176
  { .i16 = { 0, 4, 5, 7, -1, -1, -1, -1 } }, // 177
  { .i16 = { 1, 4, 5, 7, -1, -1, -1, -1 } }, // 178
  { .i16 = { 0, 1, 4, 5, 7, -1, -1, -1 } }, // 179
  { .i16 = { 2, 4, 5, 7, -1, -1, -1, -1 } }, // 180
  { .i16 = { 0, 2, 4, 5, 7, -1, -1, -1 } }, // 181
  { .i16 = { 1, 2, 4, 5, 7, -1, -1, -1 } }, // 182
  { .i16 = { 0, 1, 2, 4, 5, 7, -1, -1 } }, // 183
  { .i16 = { 3, 4, 5, 7, -1, -1, -1, -1 } }, // 184
  { .i16 = { 0, 3, 4, 5, 7, -1, -1, -1 } }, // 185
  { .i16 = { 1, 3, 4, 5, 7, -1, -1, -1 } }, // 186
  { .i16 = { 0, 1, 3, 4, 5, 7, -1, -1 } }, // 187
  { .i16 = { 2, 3, 4, 5, 7, -1, -1, -1 } }, // 188
  { .i16 = { 0, 2, 3, 4, 5, 7, -1, -1 } }, // 189
  { .i16 = { 1, 2, 3, 4, 5, 7, -1, -1 } }, // 190
  { .i16 = { 0, 1, 2, 3, 4, 5, 7, -1 } }, // 191
  { .i16 = { 6, 7, -1, -1, -1, -1, -1, -1 } }, // 192
  { .i16 = { 0, 6, 7, -1, -1, -1, -1, -1 } }, // 193
  { .i16 = { 1, 6, 7, -1, -1, -1, -1, -1 } }, // 194
  { .i16 = { 0, 1, 6, 7, -1, -1, -1, -1 } }, // 195
  { .i16 = { 2, 6, 7, -1, -1, -1, -1, -1 } }, // 196
  { .i16 = { 0, 2, 6, 7, -1, -1, -1, -1 } }, // 197
  { .i16 = { 1, 2, 6, 7, -1, -1, -1, -1 } }, // 198
  { .i16 = { 0, 1, 2, 6, 7, -1, -1, -1 } }, // 199
  { .i16 = { 3, 6, 7, -1, -1, -1, -1, -1 } }, // 200
  { .i16 = { 0, 3, 6, 7, -1, -1, -1, -1 } }, // 201
  { .i16 = { 1, 3, 6, 7, -1, -1, -1, -1 } }, // 202
  { .i16 = { 0, 1, 3, 6, 7, -1, -1, -1 } }, // 203
  { .i16 = { 2, 3, 6, 7, -1, -1, -1, -1 } }, // 204
  { .i16 = { 0, 2, 3, 6, 7, -1, -1, -1 } }, // 205
  { .i16 = { 1, 2, 3, 6, 7, -1, -1, -1 } }, // 206
  { .i16 = { 0, 1, 2, 3, 6, 7, -1, -1 } }, // 207
  { .i16 = { 4, 6, 7, -1, -1, -1, -1, -1 } }, // 208
  { .i16 = { 0, 4, 6, 7, -1, -1, -1, -1 } }, // 209
  { .i16 = { 1, 4, 6, 7, -1, -1, -1, -1 } }, // 210
  { .i16 = { 0, 1, 4, 6, 7, -1, -1, -1 } }, // 211
  { .i16 = { 2, 4, 6, 7, -1, -1, -1, -1 } }, // 212
  { .i16 = { 0, 2, 4, 6, 7, -1, -1, -1 } }, // 213
  { .i16 = { 1, 2, 4, 6, 7, -1, -1, -1 } }, // 214
  { .i16 = { 0, 1, 2, 4, 6, 7, -1, -1 } }, // 215
  { .i16 = { 3, 4, 6, 7, -1, -1, -1, -1 } }, // 216
  { .i16 = { 0, 3, 4, 6, 7, -1, -1, -1 } }, // 217
  { .i16 = { 1, 3, 4, 6, 7, -1, -1, -1 } }, // 218
  { .i16 = { 0, 1, 3, 4, 6, 7, -1, -1 } }, // 219
  { .i16 = { 2, 3, 4, 6, 7, -1, -1, -1 } }, // 220
  { .i16 = { 0, 2, 3, 4, 6, 7, -1, -1 } }, // 221
  { .i16 = { 1, 2, 3, 4, 6, 7, -1, -1 } }, // 222
  { .i16 = { 0, 1, 2, 3, 4, 6, 7, -1 } }, // 223
  { .i16 = { 5, 6, 7, -1, -1, -1, -1, -1 } }, // 224
  { .i16 = { 0, 5, 6, 7, -1, -1, -1, -1 } }, // 225
  { .i16 = { 1, 5, 6, 7, -1, -1, -1, -1 } }, // 226
  { .i16 = { 0, 1, 5, 6, 7, -1, -1, -1 } }, // 227
  { .i16 = { 2, 5, 6, 7, -1, -1, -1, -1 } }, // 228
  { .i16 = { 0, 2, 5, 6, 7, -1, -1, -1 } }, // 229
  { .i16 = { 1, 2, 5, 6, 7, -1, -1, -1 } }, // 230
  { .i16 = { 0, 1, 2, 5, 6, 7, -1, -1 } }, // 231
  { .i16 = { 3, 5, 6, 7, -1, -1, -1, -1 } }, // 232
  { .i16 = { 0, 3, 5, 6, 7, -1, -1, -1 } }, // 233
  { .i16 = { 1, 3, 5, 6, 7, -1, -1, -1 } }, // 234
  { .i16 = { 0, 1, 3, 5, 6, 7, -1, -1 } }, // 235
  { .i16 = { 2, 3, 5, 6, 7, -1, -1, -1 } }, // 236
  { .i16 = { 0, 2, 3, 5, 6, 7, -1, -1 } }, // 237
  { .i16 = { 1, 2, 3, 5, 6, 7, -1, -1 } }, // 238
  { .i16 = { 0, 1, 2, 3, 5, 6, 7, -1 } }, // 239
  { .i16 = { 4, 5, 6, 7, -1, -1, -1, -1 } }, // 240
  { .i16 = { 0, 4, 5, 6, 7, -1, -1, -1 } }, // 241
  { .i16 = { 1, 4, 5, 6, 7, -1, -1, -1 } }, // 242
  { .i16 = { 0, 1, 4, 5, 6, 7, -1, -1 } }, // 243
  { .i16 = { 2, 4, 5, 6, 7, -1, -1, -1 } }, // 244
  { .i16 = { 0, 2, 4, 5, 6, 7, -1, -1 } }, // 245
  { .i16 = { 1, 2, 4, 5, 6, 7, -1, -1 } }, // 246
  { .i16 = { 0, 1, 2, 4, 5, 6, 7, -1 } }, // 247
  { .i16 = { 3, 4, 5, 6, 7, -1, -1, -1 } }, // 248
  { .i16 = { 0, 3, 4, 5, 6, 7, -1, -1 } }, // 249
  { .i16 = { 1, 3, 4, 5, 6, 7, -1, -1 } }, // 250
  { .i16 = { 0, 1, 3, 4, 5, 6, 7, -1 } }, // 251
  { .i16 = { 2, 3, 4, 5, 6, 7, -1, -1 } }, // 252
  { .i16 = { 0, 2, 3, 4, 5, 6, 7, -1 } }, // 253
  { .i16 = { 1, 2, 3, 4, 5, 6, 7, -1 } }, // 254
  { .i16 = { 0, 1, 2, 3, 4, 5, 6, 7 } }, // 255
};

constexpr u8 lut_match_cnt[256] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3,
  4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4,
  4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4,
  5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,
  4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3,
  4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5,
  5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


/// Lookup table used to convert a 4-bit mask into a 32-bit position list
constexpr dtl::r128 lut_match_pos_4bit[16] = {
  { .i32 = { -1, -1, -1, -1 } }, // 0
  { .i32 = { 0, -1, -1, -1 } }, // 1
  { .i32 = { 1, -1, -1, -1 } }, // 2
  { .i32 = { 0, 1, -1, -1 } }, // 3
  { .i32 = { 2, -1, -1, -1 } }, // 4
  { .i32 = { 0, 2, -1, -1 } }, // 5
  { .i32 = { 1, 2, -1, -1 } }, // 6
  { .i32 = { 0, 1, 2, -1 } }, // 7
  { .i32 = { 3, -1, -1, -1 } }, // 8
  { .i32 = { 0, 3, -1, -1 } }, // 9
  { .i32 = { 1, 3, -1, -1 } }, // 10
  { .i32 = { 0, 1, 3, -1 } }, // 11
  { .i32 = { 2, 3, -1, -1 } }, // 12
  { .i32 = { 0, 2, 3, -1 } }, // 13
  { .i32 = { 1, 2, 3, -1 } }, // 14
  { .i32 = { 0, 1, 2, 3 } }, // 15
};

constexpr u8 lut_match_cnt_4bit[16] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
};


} // namespace simd

#endif // defined(__AVX2__)

#if defined(__AVX512F__)
union alignas(64)
  r512 {
  $u64 u64[8];
  $i64 i64[8];

  $u32 u32[16];
  $i32 i32[16];

  $u16 u16[32];
  $i16 i16[32];

  $u8 u8[64];
  $i8 i8[64];

  __m512i i; // integer data
  __m512  s; // single precision floating point data
  __m512d d; // double precision floating point data

  dtl::r256 r256[2];
  dtl::r128 r128[4];

//  r512(const __m512i v) : i(v) {}
//  r512() = default;
//  inline void operator=(const __m512i v) { i = v; }
};
#endif // defined(__AVX512F__)

} // namespace dtl