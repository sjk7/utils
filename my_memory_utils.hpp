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

        m_begin = (char *)mmap(nullptr, cap, flags, types, 0, 0);
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
            const auto s = my::utils::strings::system_error_string(errCode);
            if (!s.empty())
                fprintf(stderr, "VirtualAlloc failed: %s\n", s.c_str());
        }
        m_ptr = m_begin;
        m_end = m_begin + cap;
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
    Arena(size_t capacity = 4 * GB) noexcept : m_ptr(nullptr), m_begin(nullptr), m_end(nullptr)
    {
        mapmem(capacity);
    }
    ~Arena()
    {
        unmap();
    }
    void *alloc(size_t sz)
    {
        void *ret = (void *)m_ptr;
        char *newptr = m_ptr + sz;
        if (newptr >= m_end)
        {
            return nullptr;
        }
        m_ptr += sz;
        return ret;
    }
    static constexpr inline size_t KB = 1024;
    static constexpr inline size_t MB = KB * 1024;
    static constexpr inline size_t GB = MB * 1024;

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
