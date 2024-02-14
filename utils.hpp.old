#ifndef UTILS_HPP
#define UTILS_HPP
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string_view>
#include <system_error>
#include <vector>
#include <random>
#include <cassert>
namespace utils {

namespace strings {

    [[maybe_unused]] static std::string random_string(std::size_t length) {
        static const std::string CHARACTERS
            = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        static std::random_device random_device;
        static std::mt19937 generator(random_device());

        std::uniform_int_distribution<> distribution(
            0, (int)(CHARACTERS.size() - 1));

        std::string random_string;

        for (std::size_t i = 0; i < length; ++i) {
            random_string += CHARACTERS[distribution(generator)];
        }

        return random_string;
    }

    [[maybe_unused]] static inline const char* to_lower_branchless(
        std::string& sv) {
        char* d = sv.data();
        for (auto i = 0; i < (int)sv.size(); ++i) {
            unsigned char* c = (unsigned char*)&d[i];
            *c += (*c - 'A' < 26U) << 5; /* lowercase */
            //*c -= (*c-'a'<26U)<<5; /* uppercase */
            //*c ^= ((*c|32U)-'a'<26)<<5; /* toggle case */
        }
        return d;
    }
    [[maybe_unused]] static inline const char* to_upper_branchless(
        std::string& sv) {
        char* d = sv.data();
        for (auto i = 0; i < (int)sv.size(); ++i) {
            // d[i] -= 32 * (d[i] >= 'a' && d[i] <= 'z');
            d[i] -= ((unsigned char)d[i] - 'a' < 26U) << 5;

            //*c += (*c-'A'<26U)<<5; /* lowercase */
            //*c -= (*c-'a'<26U)<<5; /* uppercase */
            //*c ^= ((*c|32U)-'a'<26)<<5; /* toggle case */
        }
        return d;
    }

    [[maybe_unused]] static inline std::string_view to_upper_branchless(
        std::string_view sv) {
        char* d = (char*)sv.data();
        for (auto i = 0; i < (int)sv.size(); ++i) {
            // d[i] -= 32 * (d[i] >= 'a' && d[i] <= 'z');
            d[i] -= ((unsigned char)d[i] - 'a' < 26U) << 5;

            //*c += (*c-'A'<26U)<<5; /* lowercase */
            //*c -= (*c-'a'<26U)<<5; /* uppercase */
            //*c ^= ((*c|32U)-'a'<26)<<5; /* toggle case */
        }
        return sv;
    }

    [[maybe_unused]] static inline const char* flip_case_branchless(
        std::string& sv) {
        char* d = sv.data();
        for (auto i = 0; i < (int)sv.size(); ++i) {
            unsigned char* c = (unsigned char*)&d[i];
            *c ^= ((*c | 32U) - 'a' < 26) << 5; /* toggle case */
        }
        return d;
    }

    [[maybe_unused]] static inline void make_lower(std::string& s) {
        to_lower_branchless(s);
    }
    [[maybe_unused]] static inline void make_upper(std::string& s) {
        to_upper_branchless(s);
    }
    // make lower case chars upper case and vice versa
    [[maybe_unused]] static inline void make_flipped(std::string& s) {
        flip_case_branchless(s);
    }

    // for a faster version, see make_lower
    [[maybe_unused]] static inline std::string to_lower(const std::string& s) {
        auto data = s;
        to_lower_branchless(data);
        return data;
    }
    // for a faster version, see make_upper
    [[maybe_unused]] static inline std::string to_upper(const std::string& s) {
        auto data = s;
        to_upper_branchless(data);
        return data;
    }

    // for a faster version, see make_upper
    [[maybe_unused]] static inline std::string_view to_upper(
        const std::string_view s) {
        auto data = s;
        to_upper_branchless(data);
        return data;
    }

    // trim from start (in place)
    static inline void ltrim(std::string& s) {
        s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                [](unsigned char ch) { return !std::isspace(ch); }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                    [](unsigned char ch) { return !std::isspace(ch); })
                    .base(),
            s.end());
    }

    // trim from both ends (in place)
    [[maybe_unused]] static inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from both ends (in place)
    [[maybe_unused]] static inline std::string_view trim(std::string_view in) {
        auto left = in.begin();
        for (;; ++left) {
            if (left == in.end()) return std::string_view();
            if (!isspace(*left)) break;
        }
        auto right = in.end() - 1;
        for (; right > left && isspace(*right); --right)
            ;
        auto where_left = left - in.cbegin();
        char* p = (char*)(in.data() + where_left);
        return std::string_view(p, std::distance(left, right) + 1);
    }

    // trim from start (copying)
    [[maybe_unused]] static inline std::string ltrim_copy(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    [[maybe_unused]] static inline std::string rtrim_copy(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    [[maybe_unused]] static inline std::string trim_copy(std::string s) {
        trim(s);
        return s;
    }
    // helper for std::unordered_map in c++20 with heterogeneous lookup,
    // assuming your map is declared something like:
    // using my_hetero_type
    // = std::unordered_map<std::string, column_t, string_hash, ci_equal<>>;
    // NOTE: NOT THREAD SAFE!!
    struct string_hash_transparent_ci {
        using is_transparent = void;
        static inline std::string temp;
        [[nodiscard]] size_t operator()(char const* txt) const {
            temp = txt;
            to_upper_branchless(temp);
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(const std::string_view txt) const {
            temp.assign(txt);
            to_upper_branchless(temp);
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(std::string& txt) const {
            temp = txt;
            to_upper_branchless(temp);
            return std::hash<std::string_view>{}(txt);
        }
    };
    using key_t = std::string_view;
    template <typename T = key_t> struct transparent_string_ci_equal {
        typedef void is_transparent;
        bool operator()(const T& k1, const T& k2) const {
            return strcasecmp(k1.data(), k2.data()) == 0;
        }
    };

    template <typename... Args>
    static inline std::string concat_into(
        std::stringstream& ss, Args&&... args) {
        ((ss << args), ...);
        return std::string{ss.str()};
    }

    // way slower than cat(), but accepts any streamable type in the arguments
    // If you need faster, use concat_into and provide your own stringstream
    // object.
    template <typename... Args>
    static inline std::string concat(Args&&... args) {
        std::stringstream stream;
        return concat_into(stream, std::forward<Args>(args)...);
    }

    template <typename... ARGS>
    static inline void throw_runtime_error(ARGS&&... args) {
        const auto s = concat(std::forward<ARGS>(args)...);
        fprintf(stderr, "%s\n", s.c_str());
        fflush(stderr);
        throw std::runtime_error(s.c_str());
    }
#define THROW_ERROR(...)                                                       \
    utils::strings::throw_runtime_error(                                       \
        __FILE__, ':', __LINE__, "\r\n", __VA_ARGS__);
#define THROW_RUNTIME_ERROR THROW_ERROR

    template <typename... Args> std::size_t catSize(Args&&... args) {

        return (... + std::forward<Args>(args).size());
        // const auto totalSize = (0 + ... + strings.length());
    }

    // efficient, straight concatenate with only one allocation.
    template <typename... Args> void cat(std::string& s, Args... args) {
        (s.append(args.data(), args.size()), ...);
    }

    template <typename RESULT_TYPE>
    static inline std::vector<RESULT_TYPE> split2(
        const std::string_view haystack, const std::string_view needle = " ") {
        std::vector<RESULT_TYPE> tokens;
        auto it = haystack.begin();
        auto tok_it = it;

        while (it < haystack.end()) {
            it = std::search(it, haystack.end(), needle.begin(), needle.end());

            if (tok_it == haystack.begin() && it == haystack.end())
                return tokens; // nothing found
            tokens.emplace_back(std::move(RESULT_TYPE(tok_it, it - tok_it)));
            it += needle.size();
            tok_it = it;
        }
        return tokens;
    }

    template <typename RESULT_TYPE>
    static inline std::vector<RESULT_TYPE> split(
        std::string_view str, const std::string_view delimiters = " ") {
        std::vector<RESULT_TYPE> tokens;
        // Start at the beginning
        std::string::size_type lastPos = 0;
        // Find position of the first delimiter
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);

        // While we still have string to read
        while (std::string::npos != pos && std::string::npos != lastPos) {
            // Found a token, add it to the vector
            tokens.emplace_back(std::move(str.substr(lastPos, pos - lastPos)));
            // Look at the next token instead of skipping delimiters
            lastPos = pos + delimiters.size();
            // Find the position of the next delimiter
            pos = str.find_first_of(delimiters, lastPos);
        }

        // Push the last token
        const auto sz = str.size() - lastPos;
        if (sz && sz != std::string::npos) {
            tokens.emplace_back(str.substr(lastPos, sz));
        }
        return tokens;
    }
    [[maybe_unused]] static inline std::string replace_all_copy(
        std::string subject, const std::string& search,
        const std::string& replace) {

        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }

    // returns the number of substitutions made
    [[maybe_unused]] static inline int replace_all(std::string& subject,
        const std::string& search, const std::string& replace) {
        size_t pos = 0;
        int retval = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            // std::string where = subject.substr(pos - 1);
            subject.replace(pos, search.length(), replace);
            ++retval;
            pos += replace.length();
        }
        return retval;
    }

    // sanitize sql strings
    [[maybe_unused]] static inline void escape(std::string& s) {
        replace_all(s, "'", "''");
        replace_all(s, "\"", "\"\"");
    }
    // sanitize sql strings
    [[maybe_unused]] static inline std::string escape(std::string_view what) {
        std::string s(what);
        replace_all(s, "'", "''");
        replace_all(s, "\"", "\"\"");
        return s;
    }

    // sanitise sql strings, possibly more performant if you
    // hang on to out over multiple calls.
    [[maybe_unused]] static inline void escape(
        const std::string_view s, std::string& out) {
        out = s;
        replace_all(out, "'", "''");
        replace_all(out, "\"", "\"\"");
        return;
    }
} // namespace strings

[[maybe_unused]] static inline size_t file_get_size(std::istream& f) {
    const auto orig_pos = f.tellg();
    f.seekg(0, std::ios::end);
    size_t retval = (size_t)f.tellg();
    f.seekg(orig_pos, std::ios::beg);
    return retval;
}

[[maybe_unused]] static inline std::system_error file_open(std::fstream& f,
    std::string_view file_path, int mode, bool throw_on_fail = false) {

    f.open(file_path.data(), mode);
    if (throw_on_fail) {
        if (!f)
            throw std::system_error(errno, std::generic_category(),
                "failed to open file: " + std::string(file_path));
    }
    return std::system_error(std::error_code());
}

[[maybe_unused]] static inline void file_read_all(
    std::fstream& f, std::string& data, bool close_when_done = true) {
    data.clear();
    auto sz = file_get_size(f);
    data.resize(sz);
    assert(data.size() == sz);
    f.read(&data[0], sz);
    if (close_when_done) f.close();
}
[[maybe_unused]] static inline std::system_error file_open_and_read_all(
    std::string_view filepath, std::string& data,
    int flags = std::ios::in | std::ios::binary) {
    std::fstream f;
    auto e = file_open(f, filepath, flags, true);
    if (e.code() == std::error_code()) {
        file_read_all(f, data);
    }
    return e;
}

template <typename CONTAINER, typename COMPARE = std::less<>>
void sort(CONTAINER& c, COMPARE cmp = COMPARE()) {
    std::sort(c.begin(), c.end(), cmp);
    return;
}
template <typename CONTAINER> void sort_stable(CONTAINER& c) {
    std::stable_sort(c.begin(), c.end());
    return;
}

} // namespace utils

#ifdef UTILS_RUN_TESTS
#ifdef NDEBUG
#undef NDEBUG
#undef assert
#include <cassert> // make sure assert will work even in release build
#define NDEBUG
#else
#error "hi"
#endif

int test_split() {
    using namespace utils::strings;
    std::string s{"What a gay day!"};
    std::vector<std::string> view
        = split<std::string>(s, std::string_view{" "});
    assert(view.size() == 4);
    std::vector<std::string_view> view1
        = split<std::string_view>(s, std::string_view{" "});
    assert(view1.size() == 4);

    std::string_view gay_string{"What  a  gay  day!  "};
    view = split<std::string>(gay_string);
    assert(view.size() == 8);
    return 0;
}
int test_case() {
    using namespace utils;
    using namespace utils::strings;
    std::string s{"hello"};
    auto ret = strings::to_upper(s);
    assert(ret == "HELLO");
    ret = strings::to_lower(s);
    assert(ret == "hello");
    strings::make_upper(ret);
    assert(ret == "HELLO");

    strings::make_lower(ret);
    assert(ret == "hello");
    utils::sort(ret);
    assert(ret == "ehllo");
    utils::sort(ret, [](const auto& a, const auto& b) { return b < a; });
    assert(ret == "ollhe");
    auto r = strings::replace_all_copy(ret, "l", "z");
    assert(r == "ozzhe");
    strings::replace_all(ret, "l", "z");
    assert(ret == r);
    return 0;
}

static inline int test_more() {
    using namespace std::string_literals;
    std::string s;
    utils::strings::cat(s, "The"s, "time"s, "is"s, "now"s);
    assert(s == "Thetimeisnow");
    s = utils::strings::concat(42, 41.2, "hi");
    assert(s == "4241.2hi");

    s = utils::strings::random_string(32);
    assert(s.size() == 32);
    std::string s2 = utils::strings::random_string(32);
    assert(s2 != s);
    s2 += "\t:;!<>£";
    auto slower = utils::strings::to_lower(s2);
    auto supper = utils::strings::to_upper(s2);
    utils::strings::make_flipped(supper);
    assert(slower == supper);

    std::string flip = "aBCDEFGhijK";
    utils::strings::make_flipped(flip);
    assert(flip == "AbcdefgHIJk");
    return 0;
}

static inline int run_all_tests() {
    if (test_case()) {
        throw std::runtime_error("test_case() failed.");
    }
    if (test_split()) {
        throw std::runtime_error("test_split() failed.");
    }
    if (test_more()) {
        throw std::runtime_error("test_more() failed.");
    }
    puts("All utils tests completed successfully.\n");
    fflush(stdout);
    return 0;
}
#endif

#endif // UTILS_HPP
