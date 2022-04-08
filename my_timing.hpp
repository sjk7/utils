// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#pragma once
#ifndef SJK_TIMING_H
#define SJK_TIMING_H

#include <chrono>
#include <cstdio>
#include <string>
#include <string_view>
#include <Windows.h>

#ifdef _WIN32
#include <Windows.h> // yuk
inline void set_OS_timer() {
    static bool done = false;
    if (!done) {
        ::timeBeginPeriod(1);
    }

    done = true;
}
#pragma comment(lib, "winmm")
#else
inline void set_OS_timer() {}
#endif

namespace my {
class stopwatch {
    std::string m_sid;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;

    public:
    // nodiscard here produces (rightly) a compiler warning
    // if the user of this class forgets a variable name,
    // for example stopwatch("tick tock") < - - - B  A  D
    // So, always name itm like  stopwatch mystopwatch("tick tock")
    [[nodiscard]] stopwatch(std::string_view id)
        : m_sid(id), m_StartTime(std::chrono::high_resolution_clock::now()) {
        set_OS_timer();
    }

    auto elapsed_ms() const noexcept {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto t = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - m_StartTime);
        return t.count();
    }

    auto elapsed_ns() const noexcept {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - m_StartTime);
        return t.count();
    }
    ~stopwatch() {
        if (!m_defeat_destructor_msg) print_output();
    }

    void print_output() {
        printf("\n============================================================="
               "=\n");
        double elapsed = static_cast<double>(elapsed_ms());
        if (elapsed < 1) {
            elapsed = elapsed_ns() / 10000000000.0;
            fprintf(stdout, "%s took %fms.\n", m_sid.c_str(), elapsed);
        } else {
            fprintf(stdout, "%s took %ldms.\n", m_sid.c_str(), (long)elapsed);
        }

        printf("=============================================================="
               "\n\n");
        fflush(stdout);
    }
    auto stop() { return elapsed_ms(); }
    auto stop_and_print(bool defeat_destructor_msg = true) {
        m_defeat_destructor_msg = defeat_destructor_msg;
        auto ret = elapsed_ms();
        print_output();
        return ret;
    }
    bool m_defeat_destructor_msg = false;
};

} // namespace my

#endif // SJK_TIMING_H
