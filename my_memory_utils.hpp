#ifndef MY_MEMORY_UTILS_HPP
#define MY_MEMORY_UTILS_HPP

#include "./my_utils.hpp"
#include <cstring>

#ifndef _WIN32
#include <sys/mman.h>
#else
#include <Windows.h>
#endif

namespace my
{

// https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/Memory/reserving-and-committing-memory.md
// about 4 times the speed of malloc()
// under load on Mac Silicon.
class Arena
{

    char *m_ptr = nullptr;
    char *m_begin = nullptr;
    char *m_end = nullptr;
    void mapmem(size_t cap)
    {
        if (m_ptr != nullptr)
        {
            unmap();
        }

        int errCode = 0;
#ifndef _WIN32
        int flags = PROT_READ | PROT_WRITE;
        int types = MAP_PRIVATE | MAP_ANONYMOUS;

        m_begin = (char *)mmap(nullptr, cap, flags, types, 0, 0); // NOLINT
        if (m_begin == nullptr)
        {
            errCode = errno;
        }
#else
        m_begin = (char *)VirtualAlloc(0, cap, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!m_begin)
            errCode = GetLastError();

        assert(m_begin);

#endif
        if (errCode != 0)
        {
            const auto s = my::utils::strings::system_error_string(errCode); // NOLINT
            if (!s.empty())
            {
                (void)fprintf(stderr, "VirtualAlloc failed: %s\n", s.c_str()); // NOLINT
            }
        }
        m_ptr = m_begin;
        m_end = m_begin + cap; //NOLINT
    }
    void unmap()
    {
#ifndef _WIN32
        if (m_begin != nullptr)
        {
            munmap(m_begin, capacity());
        }
#else
        if (m_begin)
        {
            BOOL bSuccess = VirtualFree(m_begin,      // Base address of block
                                        0,            // Bytes of committed pages
                                        MEM_RELEASE); // Decommit the pages
            assert(bSuccess);
        }
#endif
        m_begin = nullptr;
        m_end = nullptr;
        m_ptr = nullptr;
    }

  public:
    explicit Arena(size_t capacity = 4 * GBytes) noexcept
    {
        mapmem(capacity);
    }
    ~Arena()
    {
        unmap();
    }
    Arena &operator=(const Arena &) = delete;
    Arena(const Arena &other) = delete;
    Arena(Arena &&other) noexcept : m_ptr(other.m_ptr), m_begin(other.m_begin), m_end(other.m_end)
    {

        other.m_ptr = nullptr;
        other.m_begin = nullptr;
        other.m_end = nullptr;
    }
    Arena &operator=(Arena &&other) noexcept
    {
        m_ptr = other.m_ptr;
        m_begin = other.m_begin;
        m_end = other.m_end;

        other.m_ptr = nullptr;
        other.m_begin = nullptr;
        other.m_end = nullptr;
        return *this;
    }

    void *alloc(size_t size)
    {
        void *ret = (void *)m_ptr;
        char *newptr = m_ptr + size; // NOLINT
        if (newptr >= m_end)
        {
            return nullptr;
        }
        m_ptr += size; // NOLINT
        return ret;
    }
    static constexpr inline size_t KBytes = 1024;
    static constexpr inline size_t MBytes = KBytes * 1024;
    static constexpr inline size_t GBytes = MBytes * 1024;

    [[nodiscard]] void *begin() const noexcept
    {
        return (void *)m_begin;
    }
    [[nodiscard]] void *end() const noexcept
    {
        return m_end;
    }
    [[nodiscard]] size_t capacity() const noexcept
    {
        return m_end - m_begin;
    }
    [[nodiscard]] size_t space() const noexcept
    {
        return m_end - m_ptr;
    }
    [[nodiscard]] size_t size() const noexcept
    {
        return m_ptr - m_begin;
    }
    void reset() noexcept
    {
        // cannot simply m_ptr = m_begin as it's super-slow to memcpy after a reset
        // in MACOS.
        mapmem(capacity());
    }
};
} // namespace my

#endif // MY_MEMORY_UTILS_HPP
