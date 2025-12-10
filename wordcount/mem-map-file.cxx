#include <string>
#include <string_view>
#include <span>
#include <filesystem>
#include <stdexcept>
#include <iterator>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <random>

#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "params.hxx"


namespace ribomation::wordcount::mem_map {
    namespace fs = std::filesystem;
    namespace r = std::ranges;
    namespace v = std::ranges::views;
    using namespace std::string_literals;
    using namespace std::string_view_literals;
    using std::string;
    using std::string_view;
    using std::span;
    using WordFreq = std::pair<string_view, unsigned>;


    class MemoryMappedFile {
        void* storage = nullptr;
        size_t size = 0;
    public:
        explicit MemoryMappedFile(const fs::path& filename) {
            const auto fd = open(filename.string().c_str(), O_RDWR);
            if (fd == -1) throw std::invalid_argument{"cannot open "s + filename.string()};

            size = fs::file_size(filename);
            storage = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            if (storage == MAP_FAILED) throw std::runtime_error{"mmap failed: "s + strerror(errno)};
            close(fd);
        }
        ~MemoryMappedFile() {
            munmap(storage, size);
        }
        [[nodiscard]] auto data() const -> std::span<char> {
            return std::span{static_cast<char *>(storage), size};
        }
        MemoryMappedFile() = delete;
        MemoryMappedFile(MemoryMappedFile const&) = delete;
        MemoryMappedFile& operator=(MemoryMappedFile const&) = delete;
    };

    class WordIterator {
        span<char> payload{};
        span<char>::iterator current_pos{};
        unsigned min_length{};
        string_view current_word{};
        bool at_end = true;

    public:
        using iterator_concept = std::input_iterator_tag;
        using iterator_category = std::input_iterator_tag;
        using value_type = string_view;
        using reference = value_type;
        using pointer = void;
        using difference_type = std::ptrdiff_t;

        WordIterator() = default;

        explicit WordIterator(span<char> payload_, unsigned min_length_)
            : payload(payload_), current_pos(payload.begin()), min_length(min_length_) {
            read_next();
        }

        reference operator*() const { return current_word; }

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
            if (a.at_end && b.at_end) return true;
            return a.at_end == b.at_end &&
                   a.payload.data() == b.payload.data() &&
                   a.current_pos == b.current_pos;
        }

        friend bool operator!=(WordIterator const& a, WordIterator const& b) {
            return !(a == b);
        }

    private:
        void read_next() {
        next_word:
            while (current_pos != payload.end() && !is_letter(*current_pos)) {
                ++current_pos;
            }

            if (current_pos == payload.end()) {
                at_end = true;
                current_word = {};
                return;
            }

            auto start = current_pos;
            while (current_pos != payload.end() && is_letter(*current_pos)) {
                *current_pos = to_lower(*current_pos);
                ++current_pos;
            }

            auto sp = span<char>(start, current_pos);;
            auto sv = string_view{sp.data(), sp.size()};
            if (sv.size() <= min_length || modern_words.contains(sv)) {
                goto next_word;
            }
            current_word = std::move(sv);
            at_end = false;
        }

        inline static std::unordered_set<string_view> const modern_words = {
            "electronic"sv, "distributed"sv, "copies"sv, "copyright"sv, "gutenberg"sv
        };

        static bool is_letter(char c) {
            const auto ch = static_cast<unsigned char>(c);
            return ('A' <= ch && ch <= 'Z')
                   || ('a' <= ch && ch <= 'z')
                   || ch == '\'';
        }

        static char to_lower(char c) {
            if ('A' <= c && c <= 'Z') {
                return static_cast<char>((c - 'A') + 'a');
            }
            return c;
        }
    };

    auto run(Params const& params) -> string {
        // --- loading words ---
        auto freqs = std::unordered_map<string_view, unsigned>{};
        auto filesize = fs::file_size(params.filename);
        auto approx_total_words = filesize / 8;
        auto approx_unique_words = approx_total_words / 4;
        freqs.reserve(approx_unique_words);

        auto file = MemoryMappedFile{params.filename};
        auto first = WordIterator{file.data(), params.min_length};
        auto last = WordIterator{};
        r::for_each(r::subrange{first, last}, [&freqs](string_view word) {
            ++freqs[word];
        });


        // --- sorting <word,count> pairs ---
        auto sortable = std::vector<WordFreq>{};
        sortable.reserve(freqs.size());
        sortable.insert(sortable.end(),
                        std::make_move_iterator(freqs.begin()), std::make_move_iterator(freqs.end()));

        auto by_count_desc = [](auto const& a, auto const& b) { return a.second > b.second; };
        r::partial_sort(sortable, sortable.begin() + params.max_words, by_count_desc);
        sortable.resize(params.max_words);


        // --- making html span tags ---
        auto max_freq = sortable.front().second;
        auto min_freq = sortable.back().second;

        class SpanTagGenerator {
            Params const& params;
            unsigned max_freq, min_freq;
            std::default_random_engine R;
            double scale;

            auto color() -> string {
                auto Byte = std::uniform_int_distribution<unsigned short>{0, 255};
                return std::format("#{:02X}{:02X}{:02X}", Byte(R), Byte(R), Byte(R));
            }

        public:
            SpanTagGenerator(Params const& params_, unsigned max_freq_, unsigned min_freq_)
                : params(params_), max_freq(max_freq_), min_freq(min_freq_) {
                scale = static_cast<double>(params.max_font - params.min_font) / (max_freq - min_freq);
                R = std::default_random_engine{std::random_device{}()};
            }

            auto operator()(WordFreq& wf) -> string {
                auto word = wf.first;
                auto freq = wf.second;
                auto size = static_cast<unsigned>((freq - min_freq) * scale + params.min_font);
                auto colr = color();
                constexpr auto fmt =
                        R"(<span style="font-size: {}px; color: {};" title="The word '{}' occurs {} times">{}</span>)";
                return std::format(fmt, size, colr, word, freq, word);
            }

            [[nodiscard]] std::default_random_engine& r() { return R; }
        };

        auto to_span_tag = SpanTagGenerator{params, max_freq, min_freq};
        r::shuffle(sortable, to_span_tag.r());

        auto html = string{};
        html.reserve(500 + (sortable.size() * 150));
        html += R"(<!DOCTYPE html>
            <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <meta name="viewport" content="width=device-width, initial-scale=1.0, shrink-to-fit=yes">
                    <title>Word Frequencies</title>
                </head>
            <body>)";
        html += std::format("<h1>The {} most frequent words in {}</h1>", params.max_words, params.filename.string());
        for (WordFreq& wf: sortable) html += to_span_tag(wf) + "\n";
        html += "</body></html>\n";

        return html;
    }
}
