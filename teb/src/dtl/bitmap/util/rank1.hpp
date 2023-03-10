#pragma once
//===----------------------------------------------------------------------===//
#include <dtl/dtl.hpp>
#include <dtl/math.hpp>

#include <boost/dynamic_bitset.hpp>

#include <string>
#include <type_traits>
#include <vector>
//===----------------------------------------------------------------------===//
namespace dtl
{
//===----------------------------------------------------------------------===//

/* Note from Marcelus: here I added get_block_granularity so teb_flat has access to
the block granularity so it can update the rank lookup tables. I made it with surf
in mind, but since every logic here has the same variable name for block granularity,
it should work.*/

/// Rank implementation that uses a precomputed lookup table (aka a dictionary).
template<typename _rank_logic>
struct rank1 {
  using rank_logic = _rank_logic;
  using word_type = typename std::remove_cv<typename rank_logic::word_type>::type;
  using size_type = typename rank_logic::size_type;

  /// Lookup table for the pre-computed rank values on block level.
  std::vector<size_type> lut;

  static constexpr u64 word_bitlength = sizeof(word_type) * 8;

  ~rank1() = default;

  /// Initializes the rank LuT based on the given bitmap. // TODO remove boost dependency
  void
  init(const boost::dynamic_bitset<word_type>& bitmap) noexcept
  {
    u64 bitmap_bitlength = bitmap.m_bits.size() * word_bitlength;
    u64 lut_entry_cnt = rank_logic::lut_entry_cnt(bitmap_bitlength);
    lut.resize(lut_entry_cnt, 0);
    rank_logic::init_inplace(
      bitmap.m_bits.data(),
      bitmap.m_bits.data() + bitmap.m_bits.size(),
      lut.data());
  }

  /// Initializes the rank LuT based on the given bitmap.
  void
  init(const word_type* const bitmap_begin,
       const word_type* const bitmap_end) noexcept
  {
    u64 bitmap_word_cnt = bitmap_end - bitmap_begin;
    u64 bitmap_bitlength = bitmap_word_cnt * word_bitlength;
    u64 lut_entry_cnt = rank_logic::lut_entry_cnt(bitmap_bitlength);
    lut.resize(lut_entry_cnt, 0);
    rank_logic::init_inplace(bitmap_begin, bitmap_end, lut.data());
  }

  /// Returns the size of the rank LuT in bytes for a bitmap of the given size.
  static constexpr u64
  estimate_size_in_bytes(u64 bitmap_size) noexcept
  {
    return rank_logic::estimate_size_in_bytes(bitmap_size);
  }

  /// Computes the rank1 of the bit at position 'idx'.
  size_type __forceinline__
  get(u64 idx, const word_type* bitmap_ptr) const noexcept
  {
    return rank_logic::get(lut.data(), idx, bitmap_ptr);
  }

  /// Computes the rank1 of the bit at position 'idx'.
  size_type __forceinline__
  operator()(u64 idx, const word_type* bitmap_ptr) const noexcept
  {
    return get(idx, bitmap_ptr);
  }


  /// Returns the size in bytes.
  u64
  size_in_bytes() const noexcept
  {
    return lut.size() * sizeof(size_type); // lut size
  }

  /// Returns the important properties in JSON.
  std::string
  info() const noexcept
  {
    return "{\"name\":"
           + std::string("\"") + rank_logic::name() + std::string("\"")
           + ",\"size\":" + std::to_string(size_in_bytes())
           + ",\"block_size\":" + std::to_string(rank_logic::block_bitlength / 8)
           + "}";
  }
  // TODO remove. rank_size should go 1 level up in the JSON
  std::string
  info(u64 bitmap_size) const noexcept
  {
    return "{\"name\":"
           + std::string("\"") + rank_logic::name() + std::string("\"")
           + ",\"size\":" + std::to_string(estimate_size_in_bytes(bitmap_size))
           + ",\"block_size\":" + std::to_string(rank_logic::block_bitlength / 8)
           + "}";
  }

  /// Returns true if the rank is inclusive, false otherwise.
  inline u1
  is_inclusive() noexcept
  {
    return rank_logic::is_inclusive;
  }

  // Returns block granularity
  u64
  get_block_bitlength()
  {
    return rank_logic::block_bitlength;
  }
};
//===----------------------------------------------------------------------===//
} // namespace dtl
