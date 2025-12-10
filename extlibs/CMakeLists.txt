Include(FetchContent)

set(BENCHMARK_USE_BUNDLED_GTEST off)
set(BENCHMARK_ENABLE_GTEST_TESTS off)
set(BENCHMARK_ENABLE_TESTING off)
set(BENCHMARK_ENABLE_LTO on)

set(CMAKE_BUILD_TYPE_SAVED ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)

FetchContent_Declare(GBench
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
)
FetchContent_MakeAvailable(GBench)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_SAVED})
