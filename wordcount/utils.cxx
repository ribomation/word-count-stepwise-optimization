#include <iostream>
#include <fstream>
#include <filesystem>
#include <print>
#include <string>
#include <chrono>
#include <functional>

#include "params.hxx"

namespace fs = std::filesystem;
namespace c = std::chrono;
using namespace std::string_literals;
using std::cout;
using std::string;
using ribomation::wordcount::Params;

void store_html(fs::path const& input_filename, string const& html_content) {
    auto html_filename = fs::path{"."} / fs::path{input_filename.stem().string() + ".html"s};
    auto html_file = std::ofstream{html_filename};
    if (not html_file) throw std::runtime_error{"cannot open outfile "s + html_filename.string()};

    html_file << html_content;
    std::println("written: {}", html_filename.string());
}

void word_count(string const& name, Params const& params, std::function<string()> const& generate_html) {
    std::println("--- WordCount - {} ---", name);
    std::println("loading {:.1f} MB from {}", fs::file_size(params.filename) / (1024.0 * 1024), params.filename.string());

    auto start = c::high_resolution_clock::now();
    auto html = generate_html();
    auto stop = c::high_resolution_clock::now();
    auto elapsed_time = c::duration_cast<c::milliseconds>(stop - start);

    store_html(params.filename, html);
    std::println("elapsed: {} ms", elapsed_time.count());
}
