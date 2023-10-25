#pragma once

#include <thread>
#include <vector>

namespace skeleton {
namespace detail {
namespace cpu {

enum class status_type { SUCCESS, FAILURE };

template <typename F, typename... Args>
status_type launcher(std::size_t chunk_size,
                     std::size_t num_items,
                     F function,
                     Args&&... args) {
  const std::size_t chunk_per_thread = chunk_size;
  const std::size_t num_chunks = (num_items + chunk_per_thread - 1) / chunk_per_thread;
  std::vector<std::thread> threads;

  for (std::size_t chunk = 0; chunk < num_chunks; chunk++) {
    std::thread thread(function, chunk, chunk_per_thread, std::forward<Args>(args)...);
    threads.push_back(std::move(thread));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  status_type result(status_type::SUCCESS);
  return result;
}

template <typename F, typename... Args>
status_type threads_launcher(std::size_t num_items,
                             std::size_t num_threads,
                             F function,
                             Args&&... args) {
  const std::size_t num_chunks = num_threads;
  const std::size_t chunk_per_thread = (num_items + num_chunks - 1) / num_chunks;
  std::vector<std::thread> threads;

  for (std::size_t chunk = 0; chunk < num_chunks; chunk++) {
    std::thread thread(function, chunk, chunk_per_thread, std::forward<Args>(args)...);
    threads.push_back(std::move(thread));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  status_type result(status_type::SUCCESS);
  return result;
}

}  // namespace cpu
}  // namespace  detail

}  // namespace skeleton