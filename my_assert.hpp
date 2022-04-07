// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

// my_assert.hpp
#pragma once
#include <string>
#include <cassert>
#include <stdexcept>
#ifdef _MSC_VER
#pragma warning(disable : 4068)
#endif

#ifdef _WIN32

#define FILENAME_MACRO(fpath)                                                  \
    (std::strrchr(fpath, '\\') ? std::strrchr(fpath, '\\') + 1 : (fpath))

#else
#define FILENAME_MACRO(fpath)                                                  \
    (std::strrchr(fpath, '/') ? std::strrchr(fpath, '/') + 1 : fpath)

#endif

namespace my {
struct assert_flags {
    static constexpr unsigned int should_throw = 1;
    static constexpr unsigned int should_abort = 2;
    unsigned int value = should_abort;
    static inline constexpr auto abort_flag() {
        return assert_flags{assert_flags::should_abort};
    }
    static inline constexpr auto throw_flag() {
        return assert_flags{assert_flags::should_throw};
    }
};
} // namespace my

#define M_Asserte(Expr_str, Expr, File, Line, Msg, Flags)                      \
    {                                                                          \
                                                                               \
        if (!(Expr)) {                                                         \
            char buf[512] = {};                                                \
            const auto file_name = FILENAME_MACRO(File);                       \
            sprintf_s(buf, 512,                                                \
                "\nAssert failed:\t%s\nExpected:\t%s\nSource:\t\t%s:%ld\n",    \
                Msg, Expr_str, file_name, Line);                               \
            fprintf(stderr, "%s", buf);                                        \
            assert(!(Expr));                                                   \
            if ((Flags).value & my::assert_flags::should_throw) {              \
                throw std::runtime_error(buf);                                 \
            }                                                                  \
            if ((Flags).value & my::assert_flags::should_abort) {              \
                abort();                                                       \
            }                                                                  \
        }                                                                      \
    }

#define M_Assert(expr_str, expr, file, line, msg)                              \
    { M_Asserte(expr_str, expr, file, line, msg, my::assert_flags()) }

// ALWAYS defined, even in release, because flags can allow throwing exception
// or aborting
#define MYASSERTE(Expr, Msg, AssertFlags)                                      \
    M_Asserte(#Expr, Expr, __FILE__, __LINE__, Msg, AssertFlags)
#define MYASSERT(Expr, Msg) M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
