#include <benchmark/benchmark.h>
#include <cctype>

static bool is_letter_orig(char c) {
    const auto ch = static_cast<unsigned char>(c);
    return std::isalpha(ch) || ch == '\'';
}

static bool is_letter_opti(char c) {
    const auto ch = static_cast<unsigned char>(c);
    return ('A' <= ch && ch <= 'Z')
           || ('a' <= ch && ch <= 'z')
           || ch == '\'';
}

static char to_lower_orig(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

static char to_lower_opti(char c) {
    if ('A' <= c && c <= 'Z') {
        return static_cast<char>((c - 'A') + 'a');
    }
    return c;
}


static void is_letter_orig_bm(benchmark::State& state) {
    for (auto _ : state) {
        auto result = is_letter_orig('A');
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(is_letter_orig_bm)->Unit(benchmark::kNanosecond)->Name("is_letter original");

static void is_letter_opti_bm(benchmark::State& state) {
    for (auto _ : state) {
        auto result = is_letter_opti('A');
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(is_letter_opti_bm)->Unit(benchmark::kNanosecond)->Name("is_letter optimized");

static void to_lower_orig_bm(benchmark::State& state) {
    for (auto _ : state) {
        auto result = to_lower_orig('A');
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(to_lower_orig_bm)->Unit(benchmark::kNanosecond)->Name("to_lower original");

static void to_lower_opti_bm(benchmark::State& state) {
    for (auto _ : state) {
        auto result = to_lower_opti('A');
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(to_lower_opti_bm)->Unit(benchmark::kNanosecond)->Name("to_lower optimized");


BENCHMARK_MAIN();
