#ifndef SJK_TIMING_H
#define SJK_TIMING_H

#include <chrono>
#include <cstdio>
#include <string>

namespace sjk {
class stopwatch
{
    std::string m_sid;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

public:
    // nodiscard here produces (rightly) a compiler warning
    // if the user of this class forgets a variable name,
    // for example stopwatch("tick tock") < - - - B  A  D
    // So, always name itm like  stopwatch mystopwatch("tick tock")
    [[nodiscard]] stopwatch(std::string_view id)
        : m_sid(id), m_StartTime(std::chrono::high_resolution_clock::now())
    {}

    auto elapsed() const noexcept
    {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto t = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_StartTime);
        return t.count();
    }
    ~stopwatch() {
        if (!m_defeat_destructor_msg) print_output();
    }

    void print_output() {
        printf("\n=================================================\n");
        fprintf(stdout, "%s took %ld ms.\n", m_sid.c_str(), (long)elapsed());
        printf("=================================================\n");
        fflush(stdout);
    }
    auto stop() { return elapsed(); }
    auto stop_and_print(bool defeat_destructor_msg = true) {
        m_defeat_destructor_msg = defeat_destructor_msg;
        auto ret = elapsed();
        print_output();
        return ret;
    }
    bool m_defeat_destructor_msg = false;
};

} // namespace sjk

#endif // SJK_TIMING_H
