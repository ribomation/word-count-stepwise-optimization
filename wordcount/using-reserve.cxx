#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <cctype>
#include "params.hxx"


namespace ribomation::wordcount::using_reserve {

    class WordIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::string;
        using reference = const std::string&;
        using pointer = const std::string *;
        using difference_type = std::ptrdiff_t;

        WordIterator() = default;

        explicit WordIterator(std::istream& is) : input{&is} {
            read_next();
        }

        reference operator*() const { return current_word; }
        pointer operator->() const { return &current_word; }

        WordIterator& operator++() {
            read_next();
            return *this;
        }

        WordIterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(WordIterator const& a, WordIterator const& b) {
            return a.input == b.input && a.at_eof == b.at_eof;
        }

        friend bool operator!=(WordIterator const& a, WordIterator const& b) {
            return !(a == b);
        }

    private:
        std::istream* input{nullptr};
        std::string current_word{};
        bool at_eof{true};

        void read_next() {
            if (input == nullptr) {
                at_eof = true;
                return;
            }

            current_word.clear();
            for (char ch; input->get(ch) && not is_letter(ch);) {}
            if (input->gcount()) input->unget();

            if (current_word.empty() && input->eof()) {
                at_eof = true;
                input = nullptr;
                return;
            }

            for (char ch; input->get(ch) && is_letter(ch);) {
                current_word.push_back(ch);
            }
            at_eof = false;
        }

        static bool is_letter(char c) {
            auto ch = static_cast<unsigned char>(c);
            return std::isalpha(ch) || ch == '\'';
        }
    };

    namespace r = std::ranges;
    namespace v = std::ranges::views;
    using std::string;

    auto run(Params const& params) -> string {
        // --- loading words ---
        auto infile = std::ifstream{params.filename};
        if (not infile) throw std::invalid_argument{"cannot open "s + params.filename.string()};

        auto freqs = std::unordered_map<string, unsigned>{};
        auto filesize = fs::file_size(params.filename);
        auto approx_total_words = filesize / 8;
        auto approx_unique_words = approx_total_words / 4;
        freqs.reserve(approx_unique_words);

        auto keep_nonsmall_words = [&params](string const& word) { return word.size() >= params.min_length; };
        auto to_lowercase = [](string const& word) {
            auto result = string{};
            result.reserve(word.size());
            for (char ch: word) {
                auto c = static_cast<unsigned char>(ch);
                result.push_back(static_cast<char>(std::tolower(c)));
            }
            return result;
        };
        auto keep_nonmodern_words = [](string const& word) {
            static auto const modern_words = std::unordered_set{
                "electronic"s, "distributed"s, "copies"s, "copyright"s, "gutenberg"s
            };
            return not modern_words.contains(word);
        };
        auto load_pipeline =
                r::subrange{WordIterator{infile}, WordIterator{}}
                | v::filter(keep_nonsmall_words)
                | v::transform(to_lowercase)
                | v::filter(keep_nonmodern_words);
        auto count_words = [&freqs](string const& word) { ++freqs[word]; };

        r::for_each(load_pipeline, count_words);

        // --- sorting <word,count> pairs ---
        using WordFreq = std::pair<string, unsigned>;
        //auto sortable = std::vector<WordFreq>{freqs.begin(), freqs.end()};
        auto sortable = std::vector<WordFreq>{};
        sortable.reserve(freqs.size());
        sortable.insert(sortable.end(), std::make_move_iterator(freqs.begin()), std::make_move_iterator(freqs.end()));

        auto by_freq_desc = [](WordFreq const& a, WordFreq const& b) { return a.second > b.second; };
        //r::sort(sortable, by_freq_desc);
        auto const N = std::min<unsigned>(params.max_words, sortable.size());
        r::partial_sort(sortable, sortable.begin() + N, by_freq_desc);
        sortable.resize(N);

        // --- making html span tags ---
        //auto items = sortable | v::take(params.max_words) | r::to<std::vector<WordFreq>>();
        auto items = std::move(sortable);

        auto max_freq = items.front().second;
        auto min_freq = items.back().second;
        auto scale = static_cast<double>(params.max_font - params.min_font) / (max_freq - min_freq);
        auto R = std::default_random_engine{std::random_device{}()};

        struct SpanTagGenerator {
            Params const& params;
            double scale;
            unsigned min_freq;
            std::default_random_engine& R;

            auto color() -> string {
                auto Byte = std::uniform_int_distribution<unsigned short>{0, 255};
                return std::format("#{:02X}{:02X}{:02X}", Byte(R), Byte(R), Byte(R));
            }

            SpanTagGenerator(Params const& params_, double scale_, unsigned min_freq_, std::default_random_engine& R_)
                : params(params_), scale(scale_), min_freq(min_freq_), R(R_) {}

            auto operator()(WordFreq& wf) -> string {
                auto word = wf.first;
                auto freq = wf.second;
                auto size = static_cast<unsigned>((freq - min_freq) * scale + params.min_font);
                auto colr = color();
                constexpr auto fmt =
                        R"(<span style="font-size: {}px; color: {};" title="The word '{}' occurs {} times">{}</span>)";
                return std::format(fmt, size, colr, word, freq, word);
            }
        };

        r::shuffle(items, R);
        auto tags = std::vector<string>{};
        tags.reserve(items.size());
        auto to_span_tag = SpanTagGenerator{params, scale, min_freq, R};
        for (WordFreq& item: items) tags.push_back(to_span_tag(item));

        // --- generating html text ---
        auto html = string{};
        html.reserve(500 + (items.size() * 150));
        html += R"(<!DOCTYPE html>
            <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <meta name="viewport" content="width=device-width, initial-scale=1.0, shrink-to-fit=yes">
                    <title>Word Frequencies</title>
                </head>
            <body>)";
        html += std::format("<h1>The {} most frequent words in {}</h1>", params.max_words, params.filename.string());
        for (string const& tag: tags) html += tag + "\n";
        html += "</body></html>\n";

        return html;
    }

}
