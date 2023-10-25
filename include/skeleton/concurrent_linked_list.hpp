
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "detail/launcher.hpp"

namespace skeleton {

template <class Key,
          class T,
          std::size_t CacheLineSize = 128,
          class Allocator = std::allocator<std::byte>>
struct concurrent_linked_list {
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const Key, T>;
  using mutable_value_type = std::pair<Key, T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using allocator_type = Allocator;

  static constexpr std::size_t cache_line_size = CacheLineSize;
  static constexpr std::size_t metadata_size =
      (sizeof(std::size_t) + sizeof(std::mutex) + sizeof(uintptr_t));
  static_assert(CacheLineSize > (metadata_size + sizeof(value_type)));
  static constexpr std::size_t node_size = cache_line_size - metadata_size;

  static constexpr std::size_t pairs_per_node = node_size / sizeof(value_type);

  struct alignas(CacheLineSize) linked_list_node {
    std::array<mutable_value_type, pairs_per_node> data;
    std::mutex node_mutex;
    std::size_t size;
    linked_list_node* next;
    linked_list_node() : size{0}, next{nullptr} {}
    void clear() {
      size = 0;
      next = nullptr;
    }
  };

  static_assert(sizeof(linked_list_node) == CacheLineSize);

  concurrent_linked_list(const Allocator& allocator = Allocator())
      : allocator_(allocator), root_{nullptr}, num_workers_{8} {
    root_ = allocate_node();
  }

  ~concurrent_linked_list() { deallocate_node(root_); }

  concurrent_linked_list(const concurrent_linked_list&) = delete;
  concurrent_linked_list(concurrent_linked_list&&) = delete;
  concurrent_linked_list& operator=(const concurrent_linked_list&) = delete;
  concurrent_linked_list& operator=(concurrent_linked_list&&) = delete;

  void insert(const value_type& pair) {
    auto current_node = root_;
    bool need_space = false;
    while (current_node != nullptr) {
      std::lock_guard<std::mutex> lock(current_node->node_mutex);
      auto size = current_node->size;
      auto next_node = current_node->next;
      if (size < pairs_per_node) {
        current_node->data[size] = pair;
        current_node->size++;
        return;
      } else if (size == pairs_per_node && next_node == nullptr) {
        linked_list_node* new_node = allocate_node();
        current_node->next = new_node;
        current_node = new_node;
      } else {
        current_node = current_node->next;
      }
    }
  }
  template <typename R>
  void insert_range(R&& rg) {
    const auto range_length = rg.end() - rg.begin();
    const std::size_t chunk_per_thread = 32;

    auto function = [&](std::size_t chunk_index,
                        std::size_t chunk_size,
                        const R&& range,
                        std::size_t num_items) {
      for (std::size_t i = 0; i < chunk_size; i++) {
        auto offset = chunk_index * chunk_size + i;
        if (offset < num_items) {
          auto absolute_index = chunk_index * chunk_size + offset;
          insert(range[absolute_index]);
        }
      }
    };

    auto result = detail::cpu::threads_launcher(range_length,  // launch parameters
                                                num_workers_,
                                                function,      // function
                                                std::ref(rg),  // args
                                                range_length);
  }

  template <typename R1, typename R2>
  void find_range(R1&& input, R2& output) {
    const auto range_length = input.end() - input.begin();

    auto function = [&](std::size_t chunk_index,
                        std::size_t chunk_size,
                        const R1&& input,
                        R2& output,
                        std::size_t num_items) {
      for (size_t i = 0; i < chunk_size; i++) {
        auto offset = chunk_index * chunk_size + i;
        if (offset < range_length) {
          output[offset] = find(input[offset]);
        }
      }
    };

    auto result = detail::cpu::threads_launcher(range_length,  // launch parameters
                                                num_workers_,
                                                function,         // function
                                                std::ref(input),  // args
                                                std::ref(output),
                                                range_length);
  }
  std::optional<mapped_type> find(const key_type& key) const {
    auto current_node = root_;
    while (current_node != nullptr) {
      std::lock_guard<std::mutex> lock(current_node->node_mutex);
      for (const auto& slot : current_node->data) {
        if (slot.first == key) {
          return std::optional<mapped_type>{slot.second};
        }
      }
      current_node = current_node->next;
    }

    return std::nullopt;
  }
  void set_num_workers(std::size_t num_workers) const { num_workers_ = num_workers; }

  void clear() {
    auto current_node = root_;
    while (current_node != nullptr) {
      auto next = current_node->next;
      if (current_node == root_) {
        current_node->clear();
      } else {
        deallocate_node(current_node);
      }
      current_node = next;
    }
  }

  [[nodiscard]] std::size_t size() const {
    auto count{0};
    auto current_node = root_;
    while (current_node != nullptr) {
      size += current_node->size;
      current_node = current_node->next;
    }
    return count;
  }

 private:
  linked_list_node* allocate_node() {
    using node_allocator_type = typename std::allocator_traits<
        allocator_type>::template rebind_alloc<linked_list_node>;
    node_allocator_type node_allocator(allocator_);

    return node_allocator.allocate(1);
  }

  void deallocate_node(linked_list_node* node) {
    using node_allocator_type = typename std::allocator_traits<
        allocator_type>::template rebind_alloc<linked_list_node>;
    node_allocator_type node_allocator(allocator_);
    node_allocator.deallocate(node, 1);
  }
  Allocator allocator_;
  linked_list_node* root_;
  std::size_t num_workers_;
};
}  // namespace skeleton
