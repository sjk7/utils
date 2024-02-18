// This is an independent project of an individual developer. Dear PVS-Studio,
//  please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
// ReSharper disable CppClangTidyMiscThrowByValueCatchByReference
#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string_view>
#include <sys/stat.h> // yup, windows has it, for checking file exist
#include <system_error>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <direct.h> // getcwd
#else
#include <unistd.h> // getcwd
#endif

#include "my_assert.hpp"
#include <map>
#include <ostream>
#include <unordered_map>

#ifdef _MSC_VER
// not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in
// mingw
// ReSharper disable once CppInconsistentNaming
#define strncasecmp _strnicmp
// ReSharper disable once CppInconsistentNaming
#define strcasecmp _stricmp
#endif

namespace my
{
static constexpr int no_error = 0;
// added for convenience due to stupid min macro in Windows.h
// that may be included before here. So always use this one for consistency
// ReSharper disable once CppInconsistentNaming
template <typename T> inline T Min(T &&t1, T &&t2) // NOLINT
{
    return (std::min)(std::forward<T>(t1), std::forward<T>(t2));
}

// ReSharper disable once CppInconsistentNaming
template <typename T> inline T Max(T &&t1, T &&t2) // NOLINT
{
    return (std::max)(std::forward<T>(t1), std::forward<T>(t2));
}

namespace utils
{

namespace strings
{

using stringvec_t = std::vector<std::string>;
using stringvecv_t = std::vector<std::string_view>;
constexpr const char PATH_SEP = '\\';

static inline std::string system_error_string(int e) // NOLINT
{
    auto ec = std::error_code(e, std::system_category()); // NOLINT
    return ec.message();
}

static inline std::string random_string(std::size_t length, int seed = -1) // NOLINT
{
    static const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwx"
                                          "yz";
    // auto seed =
    // std::chrono::system_clock::now().time_since_epoch().count();
    //  std::mt19937 generator {seed};
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    if (seed > 0)
    {
        generator = std::mt19937(seed);
    }
    std::uniform_int_distribution<size_t> distribution(size_t(0), CHARACTERS.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

[[maybe_unused]] static inline const char *to_lower_branchless(std::string &sv) // NOLINT
{
    char *d = sv.data(); // NOLINT
    for (size_t i = 0; i < sv.size(); ++i)
    {
        auto *const c = reinterpret_cast<unsigned char *>(&d[i]); // NOLINT
        *c += (*c - 'A' < 26U) << 5;                              // NOLINT
    }
    return d;
}
[[maybe_unused]] static inline const char *to_upper_branchless(std::string &sv) // NOLINT
{
    if (sv.empty())
    {
        return nullptr;
    }
    char *d = sv.data(); // NOLINT
    for (auto i = 0; i < (int)sv.size(); ++i)
    {
        d[i] -= ((unsigned char)d[i] - 'a' < 26U) << 5; //-V108 //NOLINT
    }
    return d;
}

[[maybe_unused]] static inline void to_upper_inplace(std::string_view sv) // NOLINT
{
    if (sv.empty())
    {
        return;
    }
    char *d = (char *)sv.data(); // NOLINT
    for (auto i = 0; i < (int)sv.size(); ++i)
    {
        d[i] -= ((unsigned char)d[i] - 'a' < 26U) << 5; //-V108 //NOLINT
    }
}

[[maybe_unused]] static inline const char *flip_case_branchless(std::string &sv) // NOLINT
{
    if (sv.empty())
    {
        return nullptr;
    }
    char *d = sv.data(); // NOLINT
    for (auto i = 0; i < (int)sv.size(); ++i)
    {
        auto *c = (unsigned char *)&d[i];                     //-V108 //NOLINT
        *c ^= ((*c | 32U) - 'a' < 26) << 5; /* toggle case */ // NOLINT
    }
    return d;
}

[[maybe_unused]] static inline void make_lower(std::string &s) // NOLINT
{
    const auto *dummy = to_lower_branchless(s);
    (void)dummy;
}
[[maybe_unused]] static inline void make_upper(std::string &s) // NOLINT
{
    const auto *dummy = to_upper_branchless(s);
    (void)dummy;
}
// make lower case chars upper case and vice versa
[[maybe_unused]] static inline void make_flipped(std::string &s) // NOLINT
{
    const auto *dummy = flip_case_branchless(s);
    (void)dummy;
}

// for a faster version, see make_lower
[[maybe_unused]] static inline std::string to_lower(const std::string &s) // NOLINT
{
    std::string data(s);
    const auto *dummy = to_lower_branchless(data);
    (void)dummy;
    return data;
}
// for a faster version, see make_upper
[[maybe_unused]] static inline std::string to_upper(const std::string &s) // NOLINT
{
    std::string data(s);
    to_upper_branchless(data);
    return data;
}

// trim from start (in place)
[[maybe_unused]] static inline void ltrim(std::string &s) // NOLINT
{
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch) == 0; })); // NOLINT
}

// trim from end (in place)
[[maybe_unused]] static inline void rtrim(std::string &s) // NOLINT
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch) == 0; }).base(), // NOLINT
            s.end());                                                                                          // NOLINT
}
[[maybe_unused]] static inline std::string_view ltrim(const std::string_view sv,                      // NOLINT
                                                      const std::string_view trim_what = " \t\r\v\n") // NOLINT
{
    std::string_view s{sv}; // NOLINT
    s.remove_prefix((std::min)(s.find_first_not_of(trim_what), s.size()));
    return s;
}

[[maybe_unused]] static inline std::string_view rtrim(const std::string_view sv, // NOLINT
                                                      const std::string_view trim_what = " \t\r\v\n")
{
    std::string_view s{sv}; // NOLINT
    s.remove_suffix((std::min)(s.size() - s.find_last_not_of(trim_what) - 1, s.size()));
    return s;
}

[[maybe_unused]] static inline std::string_view trim(const std::string_view sv, // NOLINT
                                                     const std::string_view trim_what = " \t\r\v\n")
{
    std::string_view ret = ltrim(sv, trim_what);
    ret = rtrim(ret, trim_what);
    return ret;
}

[[maybe_unused]] static inline bool is_numeric(std::string_view s) // NOLINT
{

    return !s.empty() &&
           std::find_if(s.begin(), s.end(), [](unsigned char c) { return !isdigit(c); }) == s.end(); // NOLINT
}

// trim from both ends (in place)
[[maybe_unused]] static inline void trim(std::string &s) // NOLINT
{
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
[[maybe_unused]] static inline std::string ltrim_copy(std::string s) // NOLINT
{
    ltrim(s);
    return s;
}

// trim from end (copying)
[[maybe_unused]] static inline std::string rtrim_copy(std::string s) // NOLINT
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
[[maybe_unused]] static inline std::string trim_copy(std::string s) noexcept // NOLINT
{
    trim(s);
    return s;
}
// trim from both ends (copying)
[[maybe_unused]] static inline std::string trim_copy(const std::string_view sv) noexcept // NOLINT
{
    std::string s(sv); // NOLINT
    trim(s);
    return s;
}

// helper for std::unordered_map in c++20 with heterogeneous lookup,
// assuming your map is declared something like:
// using my_hetero_type
// = std::unordered_map<std::string, column_t, string_hash, ci_equal<>>;
// NOTE: NOT THREAD SAFE!!
struct string_hash_transparent_ci
{
    using is_transparent = void;
    static inline std::string temp; // NOLINT
    [[nodiscard]] size_t operator()(char const *txt) const noexcept
    {

        temp = txt;
        to_upper_branchless(temp);
        return std::hash<std::string>{}(temp);
    }

    [[nodiscard]] size_t operator()(const std::string_view txt) const noexcept
    {
        temp.assign(txt);
        to_upper_branchless(temp);
        return std::hash<std::string>{}(temp);
    }

    [[nodiscard]] size_t operator()(const std::string &txt) const noexcept
    {
        temp = txt;
        to_upper_branchless(temp);
        return std::hash<std::string>{}(temp);
    }

    template <typename T> [[nodiscard]] size_t operator()(T &&) const noexcept // NOLINT
    {
        assert(0);
        return 0;
    }
};

template <class T = void> struct ci_equal_to
{
    using first_argument_type = T;
    using second_argument_type = T;
    using result_type = bool;

    [[nodiscard]] constexpr bool operator()(const T &left, const T &right) const
    {
        return left == right;
    }
};

template <> struct ci_equal_to<void>
{

    using is_transparent = int;
    // NOTE: Yes, I know left & right are COPIES: this is kinda
    // required to do a branchless comparison
    [[nodiscard]] static bool compare(std::string left, std::string right) noexcept
    {
        if (left.size() != right.size())
        {
            return false;
        }
        to_upper_branchless(left);
        to_upper_branchless(right);
        return left == right;
    }

    template <typename T1, typename T2> [[nodiscard]] bool operator()(T1 &&left, T2 &&right) const noexcept // NOLINT
    {
        return compare(std::string(left), std::string(right));
    }
};

template <class InputIt> constexpr InputIt find_if_nocase(InputIt first, InputIt last)
{

    auto p = ci_equal_to<>(); // NOLINT
    for (; first != last; ++first)
    {
        if (p(*first))
        {
            return first;
        }
    }
    return last;
}

template <class C, class T> auto contains_nocase(const C &v, const T &x) -> decltype(end(v), true) // NOLINT
{
    return end(v) != find_if_nocase(begin(v), end(v), x);
}

static inline bool compare_less_nocase_c(const char a, const char b) // NOLINT
{
    return tolower(a) < tolower(b);
}

static inline bool compare_less_nocase(const std::string_view a, const std::string_view b) noexcept // NOLINT
{
    // ReSharper disable once CppUseRangeAlgorithm
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), compare_less_nocase_c);
}

struct ci_less_lib_c
{
    using is_transparent = int;
    bool operator()(const std::string &lhs, const std::string &rhs) const noexcept
    {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
    bool operator()(const std::string_view a, const std::string_view b) const noexcept // NOLINT
    {
        return compare_less_nocase(a, b);
    }
};

#ifdef _MSC_VER
#if (_MSVC_LANG >= 202000L)
#undef HAVE_CPP_20
#define HAVE_CPP_20 1
#else
#define HAVE_CPP_20 0
#endif
#else
#if (__cplusplus >= 202000L)
#undef HAVE_CPP_20
#define HAVE_CPP_20 1
#endif
#endif
#ifdef HAVE_CPP_20
// requires c++20 //////////////////////////////////////////////////////
template <typename KEY_STRING_TYPE, typename VALUE_STRING_TYPE>
using case_insensitive_unordered_map =
    std::unordered_map<KEY_STRING_TYPE, VALUE_STRING_TYPE, string_hash_transparent_ci,
                       ci_equal_to<>>; // NOTE: unordered_map needs equal,
                                       // normal map needs less

template <typename KEY_STRING_TYPE, typename VALUE_STRING_TYPE>
using case_insensitive_map = std::map<KEY_STRING_TYPE, VALUE_STRING_TYPE, ci_less_lib_c>;
#else

template <typename K, typename V> struct case_insensitive_unordered_map : std::unordered_map<K, V>
{
    case_insensitive_unordered_map() = default;

    auto insert(std::pair<K, V> &&v) // NOLINT
    {
        static_assert(!std::is_same_v<K, decltype(v.first)>, "Only in C++20");
    }
};

template <typename K, typename V> struct case_insensitive_map : std::map<K, V>
{
    case_insensitive_map() = default;
};

#endif

template <typename... Args> static inline std::string concat_into(std::stringstream &ss, Args &&...args) // NOLINT
{
    ((ss << args), ...); // NOLINT
    return std::string{ss.str()};
}

// way slower than cat(), but accepts any streamable type in the
// arguments If you need faster, use concat_into and provide your own
// stringstream object.
template <typename... Args> static inline std::string concat(Args &&...args)
{
    std::stringstream stream;
    return concat_into(stream, std::forward<Args>(args)...);
}

template <typename... ARGS> static inline void throw_runtime_error(ARGS &&...args)
{
    const auto s = concat(std::forward<ARGS>(args)...); // NOLINT
    fprintf(stderr, "%s\n", s.c_str());
    fflush(stderr); // NOLINT
    throw std::runtime_error(s.c_str());
}
#define THROW_ERROR(...) utils::strings::throw_runtime_error(__FILE__, ':', __LINE__, "\r\n", __VA_ARGS__) // NOLINT
#define THROW_RUNTIME_ERROR THROW_ERROR

template <typename... Args> std::size_t catSize(Args &&...args)
{

    return (... + std::forward<Args>(args).size());
    // const auto totalSize = (0 + ... + strings.length());
}

// efficient, straight concatenate with only one allocation.
template <typename... Args> void cat(std::string &s, Args... args) // NOLINT
{

    (s.append(args.data(), args.size()), ...);
}

template <typename RESULT_TYPE>
static inline std::vector<RESULT_TYPE> split2(const std::string_view haystack, const std::string_view needle = " ")
{
    std::vector<RESULT_TYPE> tokens;
    const auto *it = haystack.begin(); // NOLINT
    const auto *tok_it = it;

    while (it < haystack.end())
    {
        it = std::search(it, haystack.end(), needle.begin(), needle.end());

        if (tok_it == haystack.begin() && it == haystack.end())
        {
            return tokens; // nothing found
        }
        tokens.emplace_back(std::move(RESULT_TYPE(tok_it, it - tok_it)));
        it += needle.size(); // NOLINT
        tok_it = it;
    }
    return tokens;
}

template <typename RESULT_TYPE>
static inline std::vector<RESULT_TYPE> split(std::string_view str, const std::string_view delimiters = " ")
{
    std::vector<RESULT_TYPE> tokens;
    // Start at the beginning
    std::string::size_type lastPos = 0;
    // Find position of the first delimiter
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    // While we still have string to read
    while (std::string::npos != pos && std::string::npos != lastPos)
    {
        // Found a token, add it to the vector
        tokens.emplace_back(str.substr(lastPos, pos - lastPos));
        // Look at the next token instead of skipping delimiters
        lastPos = pos + delimiters.size();
        // Find the position of the next delimiter
        pos = str.find_first_of(delimiters, lastPos);
    }

    // Push the last token
    const auto sz = str.size() - lastPos; // NOLINT
    if (sz && sz != std::string::npos)
    {
        tokens.emplace_back(str.substr(lastPos, sz));
    }
    return tokens;
}

[[maybe_unused]] static inline int HHMMSSto_secs(std::string_view sdur)
{

    auto splut = split<std::string>(sdur, ":");
    if (splut.empty())
    {
        return 0;
    }

    int cur = 0;
    int mults[] = {1, 60, 3600, 3600 * 24}; // NOLINT
    int retval = 0;
    for (const auto &s : splut) // NOLINT
    {
        const int mult_index = static_cast<int>(splut.size() - cur - 1); //-V104
        assert(mult_index >= 0);
        if (mult_index >= 4)
        { //-V112
            assert(0);
            return -1;
        }
        int this_val = std::atol(s.c_str()); // NOLINT
        this_val *= mults[mult_index];       // NOLINT
        retval += this_val;
        cur++;
    }

    return retval;
}

[[maybe_unused]] static inline std::string replace_all_copy(std::string subject, const std::string &search,
                                                            const std::string &replace)
{

    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

// returns the number of substitutions made
[[maybe_unused]] static inline size_t replace_all(std::string &subject, const std::string &search,
                                                  const std::string &replace)
{
    size_t pos = 0;
    size_t retval = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        // std::string where = subject.substr(pos - 1);
        subject.replace(pos, search.length(), replace);
        ++retval;
        pos += replace.length();
    }
    return retval;
}

// sanitize sql strings
[[maybe_unused]] static inline void escape(std::string &s) // NOLINT
{
    replace_all(s, "'", "''");
    replace_all(s, "\"", "\"\"");
}
// sanitize sql strings
[[maybe_unused]] static inline std::string escape(std::string_view what)
{
    std::string s(what); // NOLINT
    replace_all(s, "'", "''");
    replace_all(s, "\"", "\"\"");
    return s;
}

// sanitise sql strings, possibly more performant if you
// hang on to out over multiple calls.
[[maybe_unused]] static inline void escape(const std::string_view s, std::string &out) // NOLINT
{
    out = s;
    replace_all(out, "'", "''");
    replace_all(out, "\"", "\"\"");
}

template <typename N = std::string, typename V = std::string> struct name_value_pair
{
    N name;
    V value;
    inline friend std::ostream &operator<<(std::ostream &os, const name_value_pair &nvp) // NOLINT
    {
        os << nvp.name << ':' << nvp.value;
        return os;
    }
};

using stringnvp_t = name_value_pair<>;
template <typename N = std::string, typename V = std::string> using name_value_pairs_t = std::vector<stringnvp_t>;
using nvp_iterator = typename name_value_pairs_t<>::const_iterator;

[[maybe_unused]] inline nvp_iterator find_if_ci(nvp_iterator first, nvp_iterator last, const std::string_view find_what)
{

    auto p = ci_equal_to<>(); // NOLINT
    for (; first != last; ++first)
    {
        if (p(first->name, find_what))
        {
            return first;
        }
    }
    return last;
}

[[maybe_unused]] static inline nvp_iterator contains(const name_value_pairs_t<> &v,    // NOLINT
                                                     const std::string_view find_what) // NOLINT
{
    return find_if_ci(v.begin(), v.end(), find_what);
}

[[maybe_unused]] static inline name_value_pairs_t<> parse_args(const int argc, char **argv)
{
    name_value_pairs_t<> ret;
    MYASSERT(argc > 0, "no arguments to parse_args"); // NOLINT

    const stringvec_t sv(argv + 1, argv + argc); //-V104 //NOLINT

    if (sv.size() >= 2)
    {
        const auto &the_end = --sv.end();
        auto it = sv.begin(); // NOLINT
        while (it < the_end)
        {
            const auto &next = ++it;
            ret.emplace_back(stringnvp_t{*it, *next});
        }
    }

    if (ret.empty())
    {
        if (sv.size() == 1)
        {
            // the one and only argument:
            ret.emplace_back(stringnvp_t{"default", sv[0]});
        }
    }

    return ret;
}

} // namespace strings

template <typename T>
[[maybe_unused]] static int handle_max(const T ctr, const T max_ctr, const std::string &msg,
                                       const bool should_assert = true, const bool should_throw = false,
                                       const bool should_abort = true)
{

    auto myflags = my::assert_flags();
    if (should_throw)
    {
        myflags.value |= my::assert_flags::throw_flag().value;
    }
    if (should_abort)
    {
        myflags.value |= my::assert_flags::abort_flag().value;
    }

    if (ctr > max_ctr)
    {
        fprintf(stderr, "%s\n", msg.c_str()); // NOLINT
#ifdef _MSC_VER
#pragma warning(disable : 4130) // logical operation on address of string
                                // constant. Not useful!
#endif
        if (should_assert || should_abort || should_throw)
        {
            assert("limit reached" == nullptr);
            MYASSERTE(ctr < max_ctr, msg.c_str(), myflags); // NOLINT
        }
        return -1;
    }
    return no_error;
}

[[maybe_unused]] static inline size_t file_get_size(std::istream &f) // NOLINT
{
    const auto orig_pos = f.tellg();
    f.seekg(0, std::ios::end);
    const size_t retval = (size_t)f.tellg();
    f.seekg(orig_pos, std::ios::beg);
    return retval;
}

[[maybe_unused]] static inline std::string file_get_name(const std::string_view filepath,
                                                         const bool include_extn = false)
{
    std::string retval(filepath);
#ifdef _WIN32
    constexpr const char SEP = '\\';
#else
    constexpr const char SEP = '/';
#endif

    if (const auto last_slash_pos = filepath.find_last_of(SEP); last_slash_pos != std::string::npos)
    {
        retval = retval.substr(last_slash_pos + 1, retval.size() - last_slash_pos - 1);
    }
    if (!include_extn)
    {
        if (const auto last_dot_pos = retval.find_last_of('.'); last_dot_pos != std::string::npos)
        {
            retval = retval.substr(0, retval.size() - last_dot_pos);
        }
    }
    return retval;
}

[[maybe_unused]] static inline std::string get_cwd()
{

    static std::string buf(1024, 0); // NOLINT
#ifdef _WIN32
    auto *dummy = _getcwd(&buf[0], 1024);
    (void)dummy;
#else
    ::getcwd(buf.data(), 1024); // NOLINT
#endif
    const auto f = buf.find_last_not_of('\0'); // NOLINT
    if (f != std::string::npos)
    {
        buf.resize(f + 1); // NOLINT
    }
    return buf;
}

[[maybe_unused]] static inline bool file_exists(std::string_view path)
{
    assert(!path.empty());
    struct stat buffer
    {
    };
    return stat(path.data(), &buffer) == 0;
}

[[maybe_unused]] static inline std::string file_copy(std::string_view path, std::string_view extn = ".copy") // NOLINT
{

    std::string copy_to{path.data()};
    copy_to += extn.empty() ? ".copy" : extn;
    std::ifstream src(path.data(), std::ios::binary | std::ios::in);
    std::ofstream dst(copy_to, std::ios::binary | std::ios::out);

    dst << src.rdbuf();
    assert(dst && src);
    if (!dst || !src)
    {
        return {};
    }
    return copy_to;
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

[[maybe_unused]] static inline std::string get_temp_filename(const std::string &extn = ".txt", // NOLINT
                                                             const std::string &dir_name = "",
                                                             const std::string &file_prefix = "")
{
#ifdef _WIN32
    char *c_string = ::_tempnam(dir_name.c_str(), file_prefix.c_str());
#else

    char *c_string = ::tempnam(dir_name.c_str(), file_prefix.c_str());
    // char* c_string = nullptr;
    assert(0);
#endif
    MYASSERT(c_string != nullptr, "tmpnam returned null") // NOLINT
    std::string ret(c_string);
    ret.append(extn);
    free(c_string); // NOLINT
    return ret;
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

// watch it! I throw
[[maybe_unused]] static inline std::system_error file_move(const std::string &from, const std::string &to, // NOLINT
                                                           bool delete_dest = true) noexcept(false)        // NOLINT
{
    MYASSERT(!from.empty() && !to.empty(), "file_move: empty file argument") // NOLINT
    MYASSERT(from.size() > 4 && to.size() > 4,                               //-V112 //NOLINT
             "file move: suspiciously short file names")                     // NOLINT

    int r = 0; // NOLINT

    if (delete_dest && file_exists(to))
    {
        r = ::remove(to.c_str());
    }
    MYASSERT(r == 0, my::utils::strings::concat("file_move: cannot delete dest", errno).c_str()) // NOLINT
    if (r)                                                                                       // NOLINT
    {
        auto e = std::system_error(errno, std::system_category(), std::string("cannot delete file ") + from); // NOLINT
        throw e;                                                                                              // NOLINT
    }

    r = ::rename(from.c_str(), to.c_str());
    MYASSERT(r == 0, my::utils::strings::concat("file_move: cannot delete dest", errno).c_str()) // NOLINT
    if (r != 0)
    {
        auto e = std::system_error(errno, std::system_category(), std::string("cannot delete file ") + from); // NOLINT
        throw e;                                                                                              // NOLINT
    }
    return std::error_code();
}

[[maybe_unused]] static inline std::system_error file_open(std::fstream &f, // NOLINT
                                                           const std::string &file_path, std::ios_base::openmode mode,
                                                           bool throw_on_fail = false)
{
    if (file_path.empty())
    {
        auto err = std::system_error(EINVAL, std::system_category(), std::string("file_open needs a file to open!"));
        if (throw_on_fail)
        {
            throw err; //NOLINT
        }
        return err;
    }
    f.open(file_path.c_str(), mode);
    if (!f)
    {
        auto e = std::system_error(errno, std::system_category(), std::string("failed to open ") + file_path); //NOLINT
        if (throw_on_fail)
        {
            throw e; //NOLINT
        }
        else
        {
            return e;
        }
    }

    return {std::error_code()};
}

[[maybe_unused]] static inline size_t get_data_len(const std::vector<std::string_view> &d) //NOLINT
{
    size_t ret = 0;
    for (const auto &sv : d) //NOLINT
    {
        ret += sv.size();
    }
    return ret;
}

[[maybe_unused]] static inline std::system_error file_write_all(const std::vector<std::string_view> &data,
                                                                const std::string &filepath, bool throw_on_fail = false)
{

    std::fstream f; //NOLINT
    if (auto opened_for_write = file_open(f, filepath, std::ios::out | std::ios::binary, throw_on_fail);
        opened_for_write.code() != std::error_code())
    {
        return opened_for_write;
    }
    for (const auto &sv : data) //NOLINT
    {
        f.write(sv.data(), sv.length()); //NOLINT
        f.write("\r\n", 2);
        if (!f)
        {
            break;
        }
    }
    if (!f)
    {
        auto e = std::system_error(errno, std::system_category(), std::string("failed to open ") + filepath); //NOLINT
        if (throw_on_fail)
        {
            throw e; // NOLINT (misc-throw-by-value-catch-by-reference)
        }
        else
        {
            return e;
        }
    }
    return {std::error_code()};
}

[[maybe_unused]] static inline void file_read_all(std::fstream &f, std::string &data, bool close_when_done = true) //NOLINT
{
    data.clear();
    const size_t sz = file_get_size(f); //NOLINT
    data.resize(sz);
    assert(data.size() == sz);
    f.read(&data[0], sz); // NOLINT(bugprone-narrowing-conversions,
                          // cppcoreguidelines-narrowing-conversions)
    if (close_when_done) {
        f.close();
}
}

[[maybe_unused]] static inline std::system_error file_open_and_read_all(
    const std::string &filepath, std::string &data,
    const std::ios_base::openmode flags = std::ios::in | std::ios::binary)
{
    std::fstream f; //NOLINT
    auto e = file_open(f, filepath, flags, true); //NOLINT
    if (e.code() == std::error_code())
    {
        file_read_all(f, data);
    }
    return e;
}

template <typename CONTAINER, typename COMPARE = std::less<>> void sort(CONTAINER &c, COMPARE cmp = COMPARE()) //NOLINT
{
    std::sort(c.begin(), c.end(), cmp); 
}
template <typename CONTAINER> void sort_stable(CONTAINER &c) //NOLINT
{
    std::stable_sort(c.begin(), c.end());

}

} // namespace utils

#ifdef UTILS_RUN_TESTS
#ifdef NDEBUG
#undef NDEBUG
// #undef assert
#include <cassert> // make sure assert will work even in release build
#define NDEBUG
#else

#endif

int test_split()
{
    using namespace utils::strings;
    std::string s{"What a gay day!"};
    std::vector<std::string> view = split<std::string>(s, std::string_view{" "});
    assert(view.size() == 4);
    std::vector<std::string_view> view1 = split<std::string_view>(s, std::string_view{" "});
    assert(view1.size() == 4);

    std::string_view gay_string{"What  a  gay  day!  "};
    view = split<std::string>(gay_string);
    assert(view.size() == 8);
    return 0;
}
int test_case()
{
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
    utils::sort(ret, [](const auto &a, const auto &b) { return b < a; });
    assert(ret == "ollhe");
    auto r = strings::replace_all_copy(ret, "l", "z");
    assert(r == "ozzhe");
    strings::replace_all(ret, "l", "z");
    assert(ret == r);
    return 0;
}

static inline test_sv_trim()
{
    constexpr auto a = "Hello";
    constexpr auto aa = " Hello";
    constexpr auto aaa = "  Hello";

    constexpr auto b = "Hello";
    constexpr auto bb = "Hello ";
    constexpr auto bbb = "Hello  ";

    const auto c = my::utils::strings::ltrim(a);
    assert(c == "Hello");
    const auto cc = my::utils::strings::ltrim(aa);
    assert(cc == "Hello");
    const auto ccc = my::utils::strings::ltrim(aaa);
    assert(ccc == "Hello");

    const auto d = my::utils::strings::rtrim(b);
    assert(d == "Hello");
    const auto dd = my::utils::strings::rtrim(bb);
    assert(cc == "Hello");
    const auto ddd = my::utils::strings::ltrim(aaa);
    assert(ddd == "Hello");
}

static inline int test_more()
{

    auto x = my::Min(1, 2);
    assert(x == 1);
    auto y = my::Max(1, 2);
    assert(y == 2);
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

static inline int test_map()
{

    using map_t = my::utils::strings::case_insensitive_map<std::string, std::string>;
    map_t map;
    map_t map2;
    map_t map3;
    map["ffs"] = "ffs";
    assert(map.find("ffs") != map.end());
    auto found = map.find("FFS");
    assert(found != map.end());
    map2["FFS"] = "ffs";

    map3.insert(map.begin(), map.end());
    map3.insert(map2.begin(), map2.end());
    assert(map3.size() == 1);
}

static inline int run_all_tests()
{
    if (test_case())
    {
        throw std::runtime_error("test_case() failed.");
    }
    if (test_split())
    {
        throw std::runtime_error("test_split() failed.");
    }
    if (test_more())
    {
        throw std::runtime_error("test_more() failed.");
    }

    if (test_sv_trim())
    {
        throw std::runtime_error("trimming string_views failed.");
    }

    if (test_map())
    {
        throw std::runtime_error("map unique test failed.");
    }
    puts("All utils tests completed successfully.\n");
    fflush(stdout);
    return 0;
}
#endif
} // namespace my

#endif // UTILS_HPP
