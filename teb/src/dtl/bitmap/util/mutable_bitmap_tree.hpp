#pragma once
//===----------------------------------------------------------------------===//
#include "bitmap_seq_reader.hpp"
#include "bitmap_tree.hpp"

#include <dtl/bitmap/teb_flat.hpp>
#include <dtl/dtl.hpp>

#include <boost/dynamic_bitset.hpp>

#include <iomanip>
#include <immintrin.h>
//===----------------------------------------------------------------------===//
namespace dtl
{
//===----------------------------------------------------------------------===//
/// A mutable bitmap tree (MBT). An MBT is supposed to be constructed from a
/// TEB instance. It is used to update a TEB without the necessity to
/// decompress the TEB to a plain bitmap.
template<i32 optimization_level_ = 3>
class mutable_bitmap_tree
  : public bitmap_tree<optimization_level_>
{
  using size_type = teb_size_type;

  /// Keep track of the perfect levels. This value may change when the
  size_type perfect_level_cnt_;
  /// The height of the tree.
  size_type tree_height_;

public:
  /// C'tor
  explicit mutable_bitmap_tree(const dtl::teb_flat& teb) __attribute__((noinline))
    : bitmap_tree<optimization_level_>(teb.n_, false /* do not initialize the tree structure*/),
      perfect_level_cnt_(dtl::log_2(teb.implicit_inner_node_cnt_ + 1) + 1),
      tree_height_(dtl::log_2(teb.n_))
  {
    // Initialize the implicit binary tree data structure from the succinct tree
    // encoding of the given TEB.

    // Init the upper balanced part of the tree.
    const std::size_t first_explicit_node_idx = teb.implicit_inner_node_cnt_;
    this->is_inner_node_.set(
      std::size_t(0) /* + this->offset */, // set the bits < offset to 1
      first_explicit_node_idx + this->offset);
    this->is_active_node_.set(
      std::size_t(0) /* + this->offset */, // set the bits < offset to 1
      std::size_t(
        std::min(this->right_child_of(first_explicit_node_idx - 1) + 1, this->max_node_cnt_)
      ) + this->offset);

    // The current node index within the perfect binary tree data structure.
    // Recall that the node indexes in the perfect binary tree do not correspond
    // to the node indexes in the succinct tree encoding (in the lower
    // imbalanced part).
    std::size_t node_idx_impl = teb.implicit_inner_node_cnt_;

    // TODO handle the special case where only the root exists.
    // TODO ...

    assert(node_idx_impl > 0);

    dtl::bitmap_limit_seq_reader<teb_word_type> teb_tree_reader(
      teb.tree_ptr_, 0, teb.tree_bit_cnt_);

    // The parent of the current node (in the implicit tree structure).
    std::size_t parent_idx_impl = this->parent_of(node_idx_impl);

    // The current node index within the (succinctly) encoded tree.
    std::size_t node_idx_succ = teb.implicit_inner_node_cnt_;

    // Is the current node an inner node?
//    $u1 is_inner_node = false; // The first explicit node is always a leaf node.
    $u1 is_inner_node = teb_tree_reader.next(); // The first explicit node is always a leaf node.
    assert(is_inner_node == teb.is_inner_node(node_idx_succ));

    // The current label index.
    std::size_t label_idx_succ = 0;
    dtl::bitmap_limit_seq_reader<teb_word_type> teb_labels_reader(
      teb.label_ptr_, 0, teb.label_bit_cnt_);
    const std::size_t implicit_leading_label_cnt = teb.implicit_leading_label_cnt_;
    auto get_next_label_bit = [&]() {
      u1 ret_val = (label_idx_succ < implicit_leading_label_cnt)
                   ? false
                   : teb_labels_reader.next();
      ++label_idx_succ;
      return ret_val;
    };

//    $u1 label = teb.get_label_by_idx(label_idx_succ);
    $u1 label = get_next_label_bit();
    assert(label == teb.get_label_by_idx(label_idx_succ - 1));

    // Set the label in implicit binary tree.
    if (label == true) {
      this->labels_.set(node_idx_impl + this->offset);
    }

    // Determine whether the current node is a left or a right child.
    if (node_idx_impl == this->left_child_of(parent_idx_impl)) {
      // The current node is a left child.
      //
      // We add the right sibling so that both children of the current parent
      // are present in the implicit binary tree structure. Later on we iterate
      // over the parent nodes and add two children at a time.
      ++node_idx_impl;
      ++node_idx_succ;
//      is_inner_node = teb.is_inner_node(node_idx_succ);
      is_inner_node = teb_tree_reader.next();
      assert(is_inner_node == teb.is_inner_node(node_idx_succ));
      if (is_inner_node) {
        binary_tree_structure::set_inner(node_idx_impl);
      } else {
//        ++label_idx_succ;
//        label = teb.get_label_by_idx(label_idx_succ);
        label = get_next_label_bit();
        assert(label == teb.get_label_by_idx(label_idx_succ - 1));
        if (label == true) {
          this->labels_.set(node_idx_impl + this->offset);
        }
      }
    }
    assert(node_idx_impl == this->right_child_of(parent_idx_impl));

    // Iterate over the "parent" nodes, and add two children at a time.
    const auto parent_idx_impl_end =
      this->first_node_idx_at_level(this->last_level());
    while (true) {
      ++parent_idx_impl;
      parent_idx_impl = this->is_inner_node_.find_first(
                          parent_idx_impl + this->offset,
                          parent_idx_impl_end + this->offset)
                        - this->offset;
      if (parent_idx_impl >= parent_idx_impl_end) {
        // Done.
        break;
      }

      // Add the two child nodes to the binary tree structure.
      // Start with the left child.
      node_idx_impl = this->left_child_of(parent_idx_impl);
      for (std::size_t i = 0; i < 2; ++i) {
        ++node_idx_succ;
        is_inner_node = teb_tree_reader.next();
        assert(is_inner_node == teb.is_inner_node(node_idx_succ));
        if (is_inner_node) {
          binary_tree_structure::set_inner(node_idx_impl);
        } else {
          label = get_next_label_bit();
          assert(label == teb.get_label_by_idx(label_idx_succ - 1));
          if (label == true) {
            this->labels_.set(node_idx_impl + this->offset);
          }
        }
        ++node_idx_impl;
      }
    }
#ifndef NDEBUG
    this->validate_active_nodes(); // TODO remove
#endif
    this->counters_are_valid = false;
  }

  explicit mutable_bitmap_tree(const boost::dynamic_bitset<$u32>& bitmap, f64 fpr = 0.0)
    : bitmap_tree<optimization_level_>(bitmap),
      perfect_level_cnt_(1),
      tree_height_(dtl::log_2(bitmap.size())) {}

  mutable_bitmap_tree(const mutable_bitmap_tree& other) = default;
  mutable_bitmap_tree(mutable_bitmap_tree&& other) noexcept = default;
  mutable_bitmap_tree& operator=(const mutable_bitmap_tree& other) = default;
  mutable_bitmap_tree& operator=(mutable_bitmap_tree&& other) noexcept = default;
  ~mutable_bitmap_tree() override = default;

  /// Returns the value of the bit at the position pos.
  u1 __teb_inline__
  test(const std::size_t pos) const noexcept
  {
    size_type level = perfect_level_cnt_ - 1;
    const auto foo = pos >> (tree_height_ - level);

    // Determine the top-node idx.
    const auto top_node_idx_begin = (1ull << (perfect_level_cnt_ - 1)) - 1;
    size_type node_idx = top_node_idx_begin + foo;

    auto i = tree_height_ - 1 - level;
    while (!this->is_leaf_node(node_idx)) {
      u1 direction_bit = dtl::bits::bit_test(pos, i);
      node_idx = this->left_child_of(node_idx);
      node_idx += direction_bit; // right child if bit is set, left child otherwise
      --i;
    }
    return this->label_of_node(node_idx);
  }

  /// Set the bit at the given position to the given value.
  __teb_inline__ void
  set(const std::size_t pos, u1 value)
  {
    // Navigate downwards the tree until we observe a leaf node.
    size_type level = perfect_level_cnt_ - 1;
    const auto foo = pos >> (tree_height_ - level);

    // Determine the top-node idx.
    const auto top_node_idx_begin = (1ull << (perfect_level_cnt_ - 1)) - 1;
    size_type node_idx = top_node_idx_begin + foo;
    size_type i = tree_height_ - 1 - level;
    while (!this->is_leaf_node(node_idx)) {
      u1 direction_bit = dtl::bits::bit_test(pos, i);
      node_idx = this->left_child_of(node_idx);
      node_idx += direction_bit; // right child if bit is set, left child otherwise
      --i;
    }

    // If the leaf node already has the desired label, we are done.
    if (this->label_of_node(node_idx) == value) {
      return;
    }

    // If we are not yet in the last tree level, we have to break up the run
    // the current node represents.
    while (i != size_type(-1)) { // while not on the last level
      assert(this->is_leaf_node(node_idx));

      // Expand the current leaf node.
      u1 label = this->label_of_node(node_idx);
      binary_tree_structure::set_inner(node_idx);

      u1 direction_bit = dtl::bits::bit_test(pos, i);

      const auto left_child_idx = this->left_child_of(node_idx);
      const auto next_child_idx = left_child_idx + direction_bit;
      const auto sibling_of_next_child_idx = left_child_idx + !direction_bit;
      this->labels_.set(sibling_of_next_child_idx + this->offset, !value);
      node_idx = next_child_idx;
      --i;
    }
    this->labels_.set(node_idx + this->offset, value);

    // Bottom-up pruning.
    do {
      node_idx = this->parent_of(node_idx);
    } while (node_idx > 0 && try_to_collapse(node_idx));

    // Counters a no longer valid.
    this->counters_are_valid = false;
  }

  /// Toggles the bit at the given position.
  __forceinline__
  void
  toggle(const std::size_t pos)
  {
    // Navigate downwards the tree until we observe a leaf node.
    size_type level = perfect_level_cnt_ - 1;
    const auto foo = pos >> (tree_height_ - level);

    // Determine the top-node idx.
    const auto top_node_idx_begin = (1ull << (perfect_level_cnt_ - 1)) - 1;
    size_type node_idx = top_node_idx_begin + foo;
    size_type i = tree_height_ - 1 - level;
    while (!this->is_leaf_node(node_idx)) {
      u1 direction_bit = dtl::bits::bit_test(pos, i);
      node_idx = this->left_child_of(node_idx);
      node_idx += direction_bit; // right child if bit is set, left child otherwise
      --i;
    }

    u1 current_label = this->label_of_node(node_idx);
    u1 value = !current_label; // Toggle the bit.

    // If we are not yet in the last tree level, we have to break up the run
    // the current node represents.
    while (i != size_type(-1)) { // while not on the last level
      assert(this->is_leaf_node(node_idx));

      // Expand the current leaf node.
      u1 label = this->label_of_node(node_idx);
      binary_tree_structure::set_inner(node_idx);

      u1 direction_bit = dtl::bits::bit_test(pos, i);

      const auto left_child_idx = this->left_child_of(node_idx);
      const auto next_child_idx = left_child_idx + direction_bit;
      const auto sibling_of_next_child_idx = left_child_idx + !direction_bit;
      this->labels_.set(sibling_of_next_child_idx + this->offset, !value);
      node_idx = next_child_idx;
      --i;
    }
    this->labels_.set(node_idx + this->offset, value);

    // Bottom-up pruning.
    do {
      node_idx = this->parent_of(node_idx);
    } while (node_idx > 0 && try_to_collapse(node_idx));

    // Counters a no longer valid.
    this->counters_are_valid = false;
  }

private:
  /// Try to collapse the given node. Returns true if successful, false
  /// otherwise.
  u1 __forceinline__
  try_to_collapse(u64 node_idx)
  {
    const auto level = this->level_of(node_idx);
    if (level <= perfect_level_cnt_) { // FIXME ------------------------------ unclear what to do in that case
      return false;
    }
    const auto left_child_idx = this->left_child_of(node_idx);
    const auto right_child_idx = left_child_idx + 1;
    if (this->is_leaf_node(left_child_idx)
        && this->is_leaf_node(right_child_idx)) {
      const auto left_label = this->labels_.test(left_child_idx + this->offset);
      const auto right_label = this->labels_.test(right_child_idx + this->offset);
      if (left_label == right_label) {
        binary_tree_structure::set_leaf(node_idx);
        this->labels_.set(node_idx + this->offset, left_label);
        return true;
      }
    }
    return false;
  };
};
//===----------------------------------------------------------------------===//
} // namespace dtl
