
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

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
  };

  static_assert(sizeof(linked_list_node) == CacheLineSize);

  concurrent_linked_list(const Allocator& allocator = Allocator())
      : allocator_(allocator), root_{nullptr} {
    root_ = allocate_node();
  }

  ~concurrent_linked_list() { deallocate_node(root_); }

  concurrent_linked_list(const concurrent_linked_list&) = delete;
  concurrent_linked_list(concurrent_linked_list&&) = delete;
  concurrent_linked_list& operator=(const concurrent_linked_list&) = delete;
  concurrent_linked_list& operator=(concurrent_linked_list&&) = delete;

  void insert(const value_type& pair) {
    linked_list_node* current_node = root_;
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
    const std::size_t num_chunks =
        (range_length + chunk_per_thread - 1) / chunk_per_thread;

    std::vector<std::thread> threads;

    for (std::size_t chunk = 0; chunk < num_chunks; chunk++) {
      std::thread thread([chunk, chunk_per_thread, range_length, &rg, this] {
        for (size_t i = 0; i < chunk_per_thread; i++) {
          auto offset = chunk * chunk_per_thread + i;
          if (offset < range_length) {
            insert(rg[offset]);
          }
        }
      });

      threads.push_back(std::move(thread));
    }

    for (auto& thread : threads) {
      thread.join();
    }
  }
  std::optional<mapped_type> find(const key_type& key) const {
    linked_list_node* current_node = root_;
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
};
}  // namespace skeleton
