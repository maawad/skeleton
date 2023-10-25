#include <benchmark/benchmark.h>
#include <skeleton/concurrent_linked_list.hpp>
#include <thread>

// namespace {

// using Key = uint32_t;
// using Value = uint32_t;

// skeleton::unordered_map<Key, Value>* ConstructRandomMap(std::size_t size) {
//   float load_factor = 0.8;
//   std::size_t capacity = size / load_factor;
//   skeleton::unordered_map<Key, Value>* m;
//   m = new skeleton::unordered_map<Key, Value>(capacity, {-1, -1});
//   for (int i = 0; i < size; ++i) {
//     m->insert(std::make_pair(std::rand() % size, std::rand() % size));
//   }
//   return m;
// }

// }  // namespace

// // Using fixtures.
// class MapFixture : public ::benchmark::Fixture {
//  public:
//   void SetUp(const ::benchmark::State& st) BENCHMARK_OVERRIDE {
//     const std::size_t num_keys = static_cast<std::size_t>(st.range(2));
//     m = ConstructRandomMap(num_keys);
//     pairs.resize(num_keys);
//     rkg::generate_uniform_unique_pairs(keys, values, num_keys, false);
//     for (size_t i = 0; i < num_keys; i++) {
//       pairs[i] = {keys[i], values[i]};
//     }
//   }

//   void TearDown(const ::benchmark::State&) BENCHMARK_OVERRIDE { m->clear(); }

//   skeleton::unordered_map<Key, Value>* m;
//   std::vector<std::pair<Key, Value>> pairs;
//   std::vector<Key> keys;
//   std::vector<Value> values;
// };

// BENCHMARK_DEFINE_F(MapFixture, Find)(benchmark::State& state) {
//   const std::uint32_t num_threads = static_cast<std::uint32_t>(state.range(0));
//   const float load_factor = std::min(static_cast<float>(state.range(1)) / 100.f, 1.0f);
//   const std::size_t num_keys = static_cast<std::size_t>(state.range(2));

//   const std::size_t capacity = static_cast<std::size_t>(num_keys / load_factor);

//   const std::size_t work_per_thread = (num_keys + num_threads - 1) / num_threads;
//   if (num_keys >= capacity) {
//     state.KeepRunning();
//   }
//   if (work_per_thread == 0) {
//     state.KeepRunning();
//   }
//   m->clear();
//   m->resize(capacity);

//   { m->insert(pairs.begin() + begin, pairs.begin() + last); }

//   std::vector<Value> results(num_keys);

//   for (auto _ : state) {
//     m->find(keys.begin() + begin, keys.begin() + last, results.begin());
//   }

//   state.counters["load_factor"] = load_factor;
//   state.counters["find_rate"] =
//       benchmark::Counter(state.iterations() * num_keys, benchmark::Counter::kIsRate);

//   // state.SetItemsProcessed(state.iterations() * num_keys);
// }
// BENCHMARK_DEFINE_F(MapFixture, Insert)(benchmark::State& state) {
//   const std::uint32_t num_threads = static_cast<std::uint32_t>(state.range(0));
//   const float load_factor = std::min(static_cast<float>(state.range(1)) / 100.f, 1.0f);
//   const std::size_t num_keys = static_cast<std::size_t>(state.range(2));

//   const std::size_t capacity = static_cast<std::size_t>(num_keys / load_factor);

//   const std::size_t work_per_thread = (num_keys + num_threads - 1) / num_threads;
//   if (num_keys >= capacity) {
//     state.KeepRunning();
//   }
//   if (work_per_thread == 0) {
//     state.KeepRunning();
//   }
//   m->clear();
//   m->resize(capacity);

//   for (auto _ : state) {
//     std::vector<std::thread> workers;
//     for (std::size_t thread = 0; thread < num_threads; thread++) {
//       std::size_t start = thread * work_per_thread;
//       std::size_t end =
//           std::min(start + work_per_thread, static_cast<std::size_t>(num_keys));
//       workers.push_back(std::thread(
//           [&](auto begin, auto last) {
//             benchmark::DoNotOptimize(
//                 m->insert(pairs.begin() + begin, pairs.begin() + last));
//           },
//           start,
//           end));
//     }
//     benchmark::DoNotOptimize(
//         std::for_each(workers.begin(), workers.end(), [](std::thread& t) { t.join();
//         }));

//     state.PauseTiming();
//     m->clear();
//     state.ResumeTiming();
//   }

//   state.counters["load_factor"] = load_factor;
//   state.counters["insertion_rate"] =
//       benchmark::Counter(state.iterations() * num_keys, benchmark::Counter::kIsRate);

//   // state.SetItemsProcessed(state.iterations() * num_keys);
// }
// // BENCHMARK_REGISTER_F(MapFixture, Insert)
// //     ->ArgsProduct({benchmark::CreateDenseRange(40, 95, 5),
// //                    benchmark::CreateRange(1 << 10, 1 << 20, 2),
// //                    benchmark::CreateDenseRange(1, 1, 1)})
// //     ->UseRealTime();  // for threading
// // ;

// BENCHMARK_REGISTER_F(MapFixture, Insert)
//     ->ArgsProduct({benchmark::CreateDenseRange(1, 12, 1),         // number of threads
//                    benchmark::CreateDenseRange(50, 50, 5),        // load factor
//                    benchmark::CreateRange(1 << 20, 1 << 20, 2)})  // map size
//     ->UseRealTime()                                               // for threading
//     ->Unit(benchmark::kMillisecond);

// BENCHMARK_REGISTER_F(MapFixture, Find)
//     ->ArgsProduct({benchmark::CreateDenseRange(1, 12, 1),         // number of threads
//                    benchmark::CreateDenseRange(50, 50, 5),        // load factor
//                    benchmark::CreateRange(1 << 20, 1 << 20, 2)})  // map size
//     ->UseRealTime()                                               // for threading
//     ->Unit(benchmark::kMillisecond);

// // BENCHMARK_REGISTER_F(MapFixture, Lookup)->Range(1 << 3, 1 << 20);

BENCHMARK_MAIN();