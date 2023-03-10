#pragma once
//===----------------------------------------------------------------------===//
#include <dtl/dtl.hpp>

#include <type_traits>
//===----------------------------------------------------------------------===//
namespace dtl
{
namespace internal   // TODO should be dtl::bitmap::internal
{
//===----------------------------------------------------------------------===//
// Functors for bitwise operations.
//===----------------------------------------------------------------------===//
struct bitwise_and {
  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  first(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    while (!(it_a.end() || it_b.end())) {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();

      const auto begin_max = (a_begin < b_begin) ? b_begin : a_begin;
      const auto end_min = (a_end < b_end) ? a_end : b_end;
      u1 overlap = begin_max < end_min;

      if (overlap) {
        // Produce an output.
        output_pos = begin_max;
        output_length = end_min - begin_max;
        return;
      } else {
        if (a_end < b_end) {
          it_a.skip_to(b_begin);
        } else {
          it_b.skip_to(a_begin);
        }
      }
    }
    output_pos = 0;
    output_length = 0;
  }

  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  next(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    // assert a and b overlap
    {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();
      if (a_end <= b_end) {
        it_a.next();
      }
      if (b_end <= a_end) {
        it_b.next();
      }
    }

    while (!(it_a.end() || it_b.end())) {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();

      const auto begin_max = (a_begin < b_begin) ? b_begin : a_begin;
      const auto end_min = (a_end < b_end) ? a_end : b_end;
      u1 overlap = begin_max < end_min;

      if (overlap) {
        // Produce an output.
        output_pos = begin_max;
        output_length = end_min - begin_max;
        return;
      } else {
        if (a_end < b_end) {
          it_a.skip_to(b_begin);
        } else {
          it_b.skip_to(a_begin);
        }
      }
    }
    output_pos = 0;
    output_length = 0;
  }
};
//===----------------------------------------------------------------------===//
struct bitwise_or {
  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  first(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    next(it_a, it_b, output_pos, output_length);
  }

  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  next(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    while (!(it_a.end() || it_b.end())) {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();

      const auto begin_min = (a_begin > b_begin) ? b_begin : a_begin;
      const auto begin_max = (a_begin < b_begin) ? b_begin : a_begin;
      const auto end_min = (a_end < b_end) ? a_end : b_end;
      const auto end_max = (a_end > b_end) ? a_end : b_end;
      u1 contiguous = begin_max <= end_min;

      if (contiguous) {
        it_a.skip_to(end_max); // TODO not sure if that is the most efficient way
        it_b.skip_to(end_max);
        // Produce an output.
        output_pos = begin_min;
        output_length = end_max - begin_min;
        return;
      } else {
        if (a_end < b_begin) {
          it_a.next();
          // Produce an output.
          output_pos = a_begin;
          output_length = a_end - a_begin;
          return;
        } else if (b_end < a_begin) {
          it_b.next();
          // Produce an output.
          output_pos = b_begin;
          output_length = b_end - b_begin;
          return;
        }
      }
    }
    if (!it_a.end()) {
      // Produce an output.
      output_pos = it_a.pos();
      output_length = it_a.length();
      it_a.next();
      return;
    }
    if (!it_b.end()) {
      // Produce an output.
      output_pos = it_b.pos();
      output_length = it_b.length();
      it_b.next();
      return;
    }
    output_pos = 0;
    output_length = 0;
  }
};
//===----------------------------------------------------------------------===//
struct bitwise_xor {
  // TODO implement
  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  first(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    next(it_a, it_b, output_pos, output_length);
  }

  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  next(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    while (!(it_a.end() || it_b.end())) {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();

      const auto begin_min = (a_begin > b_begin) ? b_begin : a_begin;
      const auto begin_max = (a_begin < b_begin) ? b_begin : a_begin;
      const auto end_min = (a_end < b_end) ? a_end : b_end;
      const auto end_max = (a_end > b_end) ? a_end : b_end;
      u1 overlapping = begin_max < end_min;

      if (overlapping) {
        if (a_end < b_end) {
          it_a.next();
          it_b.skip_to(a_end);
        } else if (b_end < a_end) {
          it_b.next();
          it_a.skip_to(b_end);
        } else {
          it_a.next();
          it_b.next();
        }
        // Produce an output.
        output_pos = begin_min;
        output_length = begin_max - begin_min;
        if (output_length > 0) return;
      } else {
        if (a_end <= b_begin) {
          it_a.next();
          // Produce an output.
          output_pos = a_begin;
          output_length = a_end - a_begin;
          return;
        } else { /*if (b_end < a_begin)*/
          it_b.next();
          // Produce an output.
          output_pos = b_begin;
          output_length = b_end - b_begin;
          return;
        }
      }
    }
    if (!it_a.end()) {
      // Produce an output.
      output_pos = it_a.pos();
      output_length = it_a.length();
      it_a.next();
      return;
    }
    if (!it_b.end()) {
      // Produce an output.
      output_pos = it_b.pos();
      output_length = it_b.length();
      it_b.next();
      return;
    }
    output_pos = 0;
    output_length = 0;
  }
};
//===----------------------------------------------------------------------===//
struct bitwise_xor_re {
  // TODO implement
  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  first(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    next(it_a, it_b, output_pos, output_length);
  }

  template<typename iter_ta, typename iter_tb>
  static void __forceinline__
  next(iter_ta& it_a, iter_tb& it_b, $u64& output_pos, $u64& output_length)
  {
    while (!(it_a.end() || it_b.end())) {
      const auto a_begin = it_a.pos();
      const auto a_end = it_a.pos() + it_a.length();
      const auto b_begin = it_b.pos();
      const auto b_end = it_b.pos() + it_b.length();

      if (a_begin < b_begin) {
        if (a_end < b_begin) {
          it_a.next();
        } else {
          it_a.skip_to(b_end);
          it_b.next();
        }
        // Produce an output.
        output_pos = a_begin;
        output_length = b_begin - a_begin;
        return;
      } else if (b_begin < a_begin) {
        const auto end = std::min(b_end, a_begin);
        if (b_end < a_begin) {
          it_b.next();
        } else {
          it_b.skip_to(a_end);
          it_a.next();
        }
        // Produce an output.
        output_pos = b_begin;
        output_length = end - b_begin;
        return;
      } else { /* a_begin == b_begin */
        const auto end_min = (a_end < b_end) ? a_end : b_end;
        it_a.skip_to(end_min);
        it_b.skip_to(end_min);
      }
    }
    while (!it_a.end()) {
      // Produce an output.
      output_pos = it_a.pos();
      output_length = it_a.length();
      it_a.next();
      return;
    }
    while (!it_b.end()) {
      // Produce an output.
      output_pos = it_b.pos();
      output_length = it_b.length();
      it_b.next();
      return;
    }
    output_pos = 0;
    output_length = 0;
  }
};
//===----------------------------------------------------------------------===//
/// Iterator template for bitwise operations.
template<typename iter_ta, typename iter_tb, typename operation>
class bitwise_iter
{
  /// The first input iterator.
  iter_ta it_a_;
  /// The second input iterator.
  iter_tb it_b_;
  /// Points to the beginning of the current 1-fill.
  $u64 pos_ = 0;
  /// The length of the current 1-fill.
  $u64 length_ = 0;

public:
  bitwise_iter(iter_ta&& it_a, iter_tb&& it_b)
    : it_a_(std::move(it_a)), it_b_(std::move(it_b))
  {
    operation::first(it_a_, it_b_, pos_, length_);
  }

  __forceinline__
  bitwise_iter(bitwise_iter&&) = default;

  void __forceinline__
  next() noexcept __attribute__((flatten, hot))
  {
    operation::next(it_a_, it_b_, pos_, length_);
  }

  void __forceinline__
  skip_to(const std::size_t to_pos) noexcept
  {
    if (to_pos < (pos_ + length_)) {
      length_ -= to_pos - pos_;
      pos_ = to_pos;
      return;
    }
    // TODO remove condition check. - the problem here is, that internally, the iterators were already advanced
    if (to_pos > it_a_.pos()) it_a_.skip_to(to_pos);
    if (to_pos > it_b_.pos()) it_b_.skip_to(to_pos);
    operation::next(it_a_, it_b_, pos_, length_);
  }

  /// Returns true if the iterator reached the end, false otherwise.
  u1 __forceinline__
  end() const noexcept
  {
    return length_ == 0;
  }

  /// Returns the starting position of the current 1-fill.
  u64 __forceinline__
  pos() const noexcept
  {
    return pos_;
  }

  /// Returns the length of the current 1-fill.
  u64 __forceinline__
  length() const noexcept
  {
    return length_;
  }
};
//===----------------------------------------------------------------------===//
} // namespace internal
//===----------------------------------------------------------------------===//
/// Constructs a run iterator that represents the logical conjunction of the
/// given input iterators.
template<typename iter_ta, typename iter_tb>
auto __forceinline__
bitwise_and_it(iter_ta&& it_a, iter_tb&& it_b)
{
  return internal::bitwise_iter<iter_ta, iter_tb, internal::bitwise_and>(
           std::forward<iter_ta>(it_a), std::forward<iter_tb>(it_b));
};
//===----------------------------------------------------------------------===//
/// Constructs a run iterator that represents the logical disjunction of the
/// given input iterators.
template<typename iter_ta, typename iter_tb>
auto __forceinline__
bitwise_or_it(iter_ta&& it_a, iter_tb&& it_b)
{
  return internal::bitwise_iter<iter_ta, iter_tb, internal::bitwise_or>(
           std::forward<iter_ta>(it_a), std::forward<iter_tb>(it_b));
};
//===----------------------------------------------------------------------===//
/// Constructs a run iterator that represents the logical exclusive
/// disjunction of the given input iterators.
template<typename iter_ta, typename iter_tb>
auto __forceinline__
bitwise_xor_it(iter_ta&& it_a, iter_tb&& it_b)
{
  return internal::bitwise_iter<iter_ta, iter_tb, internal::bitwise_xor>(
           std::forward<iter_ta>(it_a), std::forward<iter_tb>(it_b));
};
//===----------------------------------------------------------------------===//
/// Constructs a run iterator that represents the logical exclusive
/// disjunction of the given input iterators. Applicable only in case of
/// range encoding.
template<typename iter_ta, typename iter_tb>
auto __forceinline__
bitwise_xor_re_it(iter_ta&& it_a, iter_tb&& it_b)
{
  return internal::bitwise_iter<iter_ta, iter_tb, internal::bitwise_xor_re>(
           std::forward<iter_ta>(it_a), std::forward<iter_tb>(it_b));
};
//===----------------------------------------------------------------------===//
} // namespace dtl
