include("${PROJECT_SOURCE_DIR}/cmake/FetchGoogleBench.cmake")

set(bght_includes )

set(BENCHMARKS bench_concurrent_linked_list.cpp)

foreach (benchmark_src ${BENCHMARKS})
    get_filename_component(benchmark_name ${benchmark_src} NAME_WE)
    add_executable(${benchmark_name} ${benchmark_src})
    target_link_libraries(${benchmark_name} PRIVATE skeleton::skeleton benchmark::benchmark)
endforeach()
