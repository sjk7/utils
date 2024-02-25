
#include "../../utils/my_memory_utils.hpp"
#include "../../utils/my_timing.hpp"
#include "../../utils/my_utils.hpp"
#include <cstdio>
#include <string>
#include <vector>

using std::vector;

using namespace std;
using namespace my;

vector<std::string> make_random_strings(size_t howMany, size_t string_len = 32) // NOLINT
{
    std::vector<std::string> strings;
    printf("Making %zu random strings of length: %zu ...\n", howMany, string_len); // NOLINT
    strings.reserve(howMany);
    for (size_t i = 0; i < howMany; ++i)
    {
        strings.push_back(utils::strings::random_string(string_len));
    }
    printf("Making %zu random strings, complete.\n", howMany); // NOLINT
    return strings;
}

auto test_arena(const std::vector<std::string> &v, Arena **a, bool &reset) // NOLINT
{
    volatile uint64_t ret = 0;
    const auto start_time = my::stopwatch::now_ms();

    auto l = (size_t)0;     // NOLINT
    for (const auto &s : v) // NOLINT
    {
        l = s.size();
        volatile char *ptr = (char *)(*a)->alloc(l + 1); // NOLINT
        if (ptr == nullptr)
        {
            printf("******* Arena out of space. Only %zu bytes free. Resetting it. " // NOLINT
                   "*******\n",
                   (*a)->space()); // NOLINT
            reset = true;
            // delete (*a);
            //(*a) = new Arena();
            (*a)->reset();
            continue;
        }
        if (ptr != nullptr)
        {
            memcpy((char *)ptr, s.c_str(), l + 1);
        }
        ret += strlen((char *)ptr); // need this to avoid optimising away
    };

    // printf("space() in Arena ================== %zu\n", (*a)->space());
    return my::stopwatch::now_ms() - start_time;
}

auto test_malloc(const std::vector<std::string> &v) //NOLINT
{

    volatile uint64_t ret = 0;
    const auto start_time = my::stopwatch::now_ms();
    char *firstptr = nullptr;
    for (const auto &s : v) //NOLINT
    { // NOLINT

        const auto l = s.size(); //NOLINT
        volatile char *ptr = (char *)malloc(l + 1); // NOLINT
        memcpy((void *)ptr, s.c_str(), l + 1);
        ret -= strlen((char *)ptr); // do not optimise me away!
        if (firstptr == nullptr)
        {
            firstptr = (char *)ptr;
        }
    }
    // free(firstptr); <-- Arena is not free'ing every time, and neither are we
    return my::stopwatch::now_ms() - start_time;
}

int main()
{
    puts("App started. It deliberately leaks memory.\nDon't worry about "
         "it!\nPlease wait ...");
    auto v = make_random_strings(3'000'000); //NOLINT

    int64_t arena_time = 0;
    int64_t malloc_time = 0;
    uint64_t ret = 0;
    int64_t notify_period_ms = 250; //NOLINT
    auto time_marker = my::stopwatch::now_ms();

    std::string dot = ".";
    Arena *a = nullptr; //NOLINT
    a = new Arena(); //NOLINT

    int arena_reset_at = 0;
    int loop_max = 20; //NOLINT
    bool reset = false;
    for (int i = 0; i < loop_max; ++i)
    {
        if (i == loop_max - 1 && arena_reset_at == 0)
        {
            puts("Adding more iterations: arena has not yet reset()");
            loop_max += 20; //NOLINT
        }
        if (my::stopwatch::now_ms() - notify_period_ms >= time_marker)
        {
            time_marker = my::stopwatch::now_ms();
            printf("%s\n", dot.c_str()); //NOLINT
            dot += '.';
        }
        arena_time += test_arena(v, &a, reset);
        if (reset)
        {
            if (arena_reset_at == 0)
            {
                puts("Will make sure arena resets again, at least one more time");
                arena_reset_at = i;
                loop_max = i + arena_reset_at + 2;
            }
        }
        // printf("ret a: %zu\n", (size_t)ret);

        malloc_time += test_malloc(v);
        // printf("ret b: %zu\n", (size_t)ret);
    }

    puts(" ---------- Previous results, for comparison --------");
    printf("\nARM64 build, Win11 UTM VM on Apple " //NOLINT
           "Silicon:   Arena 1.5-2x\n");
    printf("ARM64 build,  in MACOS on Apple " //NOLINT
           "Silicon:   Arena 1.8-2.2x +\n");

    printf("x86_64 build, in MACOS on Apple " //NOLINT
           "Silicon (native):   Arena 1.8x +\n");

    puts("\n\n ------------------- Results -------------------");
    printf("Total Arena   execution time (ms): %lld\n", arena_time); //NOLINT
    printf("Total malloc  execution time (ms): %lld\n", malloc_time); //NOLINT
    float ratio = 1;
    if (malloc_time < arena_time)
    {
        ratio = (float)arena_time / (float)malloc_time;
        printf("Malloc faster than arena (%fx)\n", ratio); //NOLINT
    }
    else
    {
        ratio = (float)malloc_time / (float)arena_time;
        printf("Arena faster than malloc (%fx)\n", ratio); //NOLINT
    }

    delete a; //NOLINT

    printf("\nAll done!\n"); //NOLINT
    return (int)ret;

    
    Arena myArena;
    printf("Arena capacity() is %zu bytes\n", myArena.capacity()); //NOLINT
    assert(myArena.capacity() > 0);

    size_t old_free = myArena.space();
    char* strptr = (char*)myArena.alloc(256); // NOLINT
    size_t new_free = myArena.space();
    assert(old_free - new_free == 256);
    assert(myArena.size() == 256);
    constexpr const char* greet = "Hello, nobba!";
    strcpy(strptr, greet);

    assert(strptr);
    puts(strptr);
    return 0;
}


