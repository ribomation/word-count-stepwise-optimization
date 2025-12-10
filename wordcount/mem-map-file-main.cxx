#include <string>
#include <functional>
#include "params.hxx"

using namespace std::string_literals;
using std::string;
using ribomation::wordcount::Params;

extern void word_count(string const& name, Params const& params, std::function<string()> const& generate_html);

namespace ribomation::wordcount::mem_map {
    extern auto run(Params const& P) -> std::string;
}

int main(int argc, char* argv[]) {
    auto params = Params{};
    params.parse(argc, argv);

    word_count("Using reserve and partial_sort"s, params, [&params]() {
        return ribomation::wordcount::mem_map::run(params);
    });
}
