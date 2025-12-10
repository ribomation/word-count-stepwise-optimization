#include <benchmark/benchmark.h>
#include "params.hxx"

namespace ribomation::wordcount::baseline {
    extern auto run(Params const& P) -> std::string;
}
namespace ribomation::wordcount::using_reserve {
    extern auto run(Params const& P) -> std::string;
}
namespace ribomation::wordcount::char_fn {
    extern auto run(Params const& P) -> std::string;
}
namespace ribomation::wordcount::mem_map {
    extern auto run(Params const& P) -> std::string;
}
using ribomation::wordcount::Params;


static void baseline_bm(benchmark::State& state) {
    auto params = Params{};
    for (auto _ : state) {
        auto html = ribomation::wordcount::baseline::run(params);
        benchmark::DoNotOptimize(html);
    }
}
BENCHMARK(baseline_bm)->Unit(benchmark::kMillisecond)->Name("Baseline");

static void reserve_bm(benchmark::State& state) {
    auto params = Params{};
    for (auto _ : state) {
        auto html = ribomation::wordcount::using_reserve::run(params);
        benchmark::DoNotOptimize(html);
    }
}
BENCHMARK(reserve_bm)->Unit(benchmark::kMillisecond)->Name("Using reserve()");

static void characters_bm(benchmark::State& state) {
    auto params = Params{};
    for (auto _ : state) {
        auto html = ribomation::wordcount::char_fn::run(params);
        benchmark::DoNotOptimize(html);
    }
}
BENCHMARK(characters_bm)->Unit(benchmark::kMillisecond)->Name("Opt char fns");

static void memmap_bm(benchmark::State& state) {
    auto params = Params{};
    for (auto _ : state) {
        auto html = ribomation::wordcount::mem_map::run(params);
        benchmark::DoNotOptimize(html);
    }
}
BENCHMARK(memmap_bm)->Unit(benchmark::kMillisecond)->Name("Memory-mapped file");

BENCHMARK_MAIN();
