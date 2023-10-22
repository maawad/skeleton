#include <skeleton/concurrent_linked_list.hpp>

// std::
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char** argv) {
  using K = uint32_t;
  using V = uint32_t;
  static constexpr auto NodeSize = 128;
  skeleton::concurrent_linked_list<K, V, NodeSize> list;

  const std::size_t num_keys = 12;
  std::vector<K> keys(num_keys);
  std::vector<V> values(num_keys);
  std::iota(keys.begin(), keys.end(), 0);
  std::iota(values.begin(), values.end(), num_keys);

  std::vector<std::pair<K, V>> pairs;
  pairs.reserve(num_keys);

  std::transform(keys.begin(),
                 keys.end(),
                 values.begin(),
                 std::back_inserter(pairs),
                 [](const K& key, const V& value) { return std::make_pair(key, value); });

  list.insert_range(pairs);

  for (const auto& key : keys) {
    auto result = list.find(key);
    if (result.has_value()) {
      std::cout << "Key: " << key << ", Value: " << result.value() << std::endl;
    } else {
      std::cout << "Key: " << key << " not found." << std::endl;
    }
  }
}
