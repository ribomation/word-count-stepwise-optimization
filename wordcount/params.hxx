#pragma once
#include <filesystem>
#include <string>

namespace ribomation::wordcount {
    namespace fs = std::filesystem;
    using namespace std::string_literals;

    struct Params {
        fs::path basedir = fs::path{"../.."};
        fs::path filename = basedir / fs::path{"data/shakespeare.txt"};
        unsigned min_length = 6U;
        unsigned max_words = 100U;
        unsigned max_font = 200U;
        unsigned min_font = 40U;

        void parse(int argc, char* argv[]) {
            for (auto k = 1; k < argc; ++k) {
                auto arg = std::string{argv[k]};
                if (arg == "--file"s) {
                    filename = fs::path{argv[++k]};
                } else if (arg == "--min"s) {
                    min_length = std::stoul(argv[++k]);
                } else if (arg == "--max"s) {
                    max_words = std::stoul(argv[++k]);
                }
            }
        }
    };

}
