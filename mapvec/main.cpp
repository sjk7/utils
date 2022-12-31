
#include "../../utils/my_timing.hpp"
#include "../../utils/my_utils.hpp"
#include "mapvec.h"
#include <random>
#include <unordered_map>
#include <algorithm>

using namespace std;

void test_as_keys(const size_t);
void test_strings(const size_t, bool sso = false);
int main() {
    cout << "Size of float: " << sizeof(float) << endl;
    cout << "Size of double: " << sizeof(double) << endl;
    SortedVector<int> sv;
    auto insertResult = sv.insert(11);
    assert(insertResult != sv.end());
    // assert(insertResult.second);
    assert(sv.size() == 1);
    assert(sv.find(11) != sv.end());
    assert(*(sv.find(11)) == 11);
    auto ins = sv.insert(11);
    // assert(ins.second == false);
    assert(*ins == 11);
    assert(sv.size() == 1);
    cout << endl;

    {
        SortedVector<int, MultiKeysPolicy> sv;
        auto ins = sv.insert(77);
        assert(ins.Inserted() && ins.Where() != sv.end());
        assert(sv.size() == 1);
        ins = sv.insert(77);
        assert(ins.Inserted() && ins.Where() != sv.end());
        assert(sv.size() == 2);
        auto it = std::equal_range(sv.begin(), sv.end(), 77);
        assert(*it.first == 77);
        ++it.first;
        assert(*it.first == 77);
        ++it.first;
        assert(it.first == sv.end());
    }

#ifdef NDEBUG
    static const int N = 100'000;

#else
    static const int N = 50'000;
#endif

    cout << "******************* TESTING " << N << " SHORT STRINGS"
         << "*****************\n\n ";
    test_strings(N, true);
    cout << "******************* TESTING " << N << " LONG STRINGS"
         << "*****************\n\n ";
    test_strings(N, false);
}

std::vector<int> make_random_ints(const size_t N) {
    std::random_device
        rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
    // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, (int)(N - 1));
    std::vector<int> vals;
    vals.reserve(N);

    for (int i = 0; i < N; i++) {
        const auto r = i; // distrib(gen);
        vals.push_back(r);
    }
    return vals;
}

void test_strings(const size_t n, bool sso) {
    using str_um_t = std::unordered_map<std::string, std::string>;
    using sm_t = SteveMap<std::string, std::string>;
    str_um_t um;
    sm_t sm;
    std::vector<std::string> random_strings;
    random_strings.reserve(n);
    cout << "Creating " << n << " random strings... " << endl;
    for (int i = 0; i < n; ++i) {
        random_strings.push_back(
            my::utils::strings::random_string(sso ? 10 : 64));
    }

    cout << "Creating " << n << " random (unfindable) strings... " << endl;
    std::vector<std::string> unfindable;
    unfindable.reserve(n);
    for (int i = 0; i < n; ++i) {
        unfindable.push_back(my::utils::strings::random_string(sso ? 6 : 32));
    }

    {
        int ctr = 0;
        my::stopwatch sw("Adding items to unordered_map");
        for (auto&& s : random_strings) {
            um.insert({s, s});
        }
        assert(um.size() == random_strings.size());
    }

    {
        int ctr = 0;
        sm.reserve(random_strings.size());
        my::stopwatch sw("Adding items to SteveMap");
        // sm.insert_range(random_strings); <-- also works, but perhaps unfair
        // comparison?
        for (auto&& s : random_strings) {
            auto ret = sm.insert({s, s});
            assert(ret != sm.end());
            ++ctr;
        }
        assert(sm.size() == random_strings.size());
        assert(sm.values().size() == random_strings.size());
        assert(sm.keys().size() == random_strings.size());
    }

    {
        int ctr = 0;
        sm.reserve(random_strings.size());
        my::stopwatch sw(
            "Adding existing items to SteveMap (nothing should happen)");
        using pair_type = std::pair<std::string, std::string>;
        std::vector<pair_type> v;
        v.reserve(sm.size());
        for (auto&& s : random_strings) {
            pair_type p{s, s};
            v.push_back(p);
        }
        sm.insert_range(v);

        assert(sm.size() == random_strings.size());
        assert(sm.values().size() == random_strings.size());
        assert(sm.keys().size() == random_strings.size());
    }

    {
        int ctr = 0;
        my::stopwatch sw(
            "Adding existing items to UnorderedMap (nothing should happen)");
        // sm.insert_range(random_strings); <-- also works, but perhaps unfair
        // comparison?
        for (auto&& s : random_strings) {
            um.insert({s, s});
        }
        assert(um.size() == random_strings.size());
    }

    // check they won't go in a second time:

    cout << "Shuffling the keys so they are not in the order we added them to "
            "the maps ..."
         << endl;
    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(random_strings), std::end(random_strings), rng);

    auto ints = make_random_ints(n);
    {
        volatile int fake = 0;
        my::stopwatch sw("Finding random (existing) keys in unordered_map");
        for (auto i : ints) {
            auto found = um.find(random_strings[i]);
            assert(found != um.end());
            if (found != um.end()) {
                fake = fake + 1;
            }
        }
        assert((size_t)fake == ints.size());
        // cout << "fake output " << fake << endl;
    }

    {
        int ctr = 0;
        volatile int fake = 0;
        my::stopwatch sw("Finding random (existing) keys in stevemap");
        const auto& keys = sm.keys();
        for (int i = 0; i < ints.size(); ++i) {
            auto found = sm.find(keys[i].first);
            assert(found != sm.end());
            if (found != sm.end()) {

                fake = fake + 1;
            }
            ++ctr;
        }
        assert((size_t)fake == ints.size());
        // cout << "fake output " << fake << endl;
    }

    auto isize = (int)um.size();
    {
        volatile int fake = 0;
        int idx = 0;

        my::stopwatch sw("Finding random (non-existing) keys in unordered_map");
        for (int i = isize; i < isize + n; i++) {
            auto found = um.find(unfindable[idx++]);
            if (found != um.end()) {
                fake = fake + 1;
            }
        }
        assert((size_t)fake == 0);
        // cout << "fake output " << fake << endl;
    }

    {
        volatile int fake = 0;
        int idx = 0;
        my::stopwatch sw("Finding random (non-existing) keys in Stevemap");
        for (int i = isize; i < isize + n; i++) {
            auto found = sm.find(unfindable[idx++]);
            if (found != sm.end()) {
                fake = fake + 1;
            }
        }
        assert((size_t)fake == 0);
        // cout << "fake output " << fake << endl;
    }
}

template <typename T> void add_stuff(T& t, std::string what, const int N) {
    my::stopwatch sw(what);
    t.reserve(N);

    for (int i = 0; i < N; i++) {
        t.insert({i, i});
    }
}

template <typename K, typename V>
void add_stuff(SteveMap<K, V>& t, std::string what, const int N) {
    my::stopwatch sw(what);
    t.reserve(N);

    std::random_device
        rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
    // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, N - 1);
    std::vector<V> vals;
    vals.reserve(N);

    for (int i = 0; i < N; i++) {
        const auto r = i; // distrib(gen);
        vals.push_back(r);
    }

    t.insert_range(vals);
}

template <typename K, typename V>
void test_find(std::unordered_map<K, V>& m, SteveMap<K, V>& sm) {
    const int N = m.size();
    std::random_device
        rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());
    // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, m.size() - 1);
    volatile unsigned int fake_total = 0;

    {
        my::stopwatch sw("Finding randomly in " + std::to_string(N)
            + " unordered_map items");
        for (int i = 0; i < N; i++) {
            int r = distrib(gen);
            auto itfound = m.find(r);
            assert(itfound != m.end());
            auto v = *itfound;
            fake_total += v.second;
        }
    }

    // cout << fake_total << '\r';
    {
        my::stopwatch sw(
            "Finding randomly in " + std::to_string(N) + " SteveMap items");
        for (int i = 0; i < N; i++) {
            int r = distrib(gen);
            auto pr = sm.find(r);
            auto v = pr.second;
            fake_total += v;
        }
    }
    // cout << fake_total << '\r';
}

template <typename T> void iterate_over(const T& t, const char* what) {
    volatile unsigned long fake = 0;
    my::stopwatch sw(what);
    for (const auto& pr : t) {
        fake += pr.second;
    }
    // cout << "iterator fake is: " << '\r';
    //;
}

template <typename T> void map_sanity(const T& t) {
    volatile unsigned long fake = 0;

    for (const auto& pr : t) {
        bool b = pr.first == pr.second;
        assert(b);
        fake += b;
    }

    // std::cout << fake << '\r';
    //;
}

/*/
    void test_as_keys(size_t N) {
        SteveMap<int, int> v;
        std::unordered_map<int, int> m;

        add_stuff(v, "adding " + std::to_string(N) + " to SteveMap", N);
        add_stuff(m, "adding " + std::to_string(N) + " to map", N);

        test_find(m, v);
        iterate_over(v, "Iterate over SteveMap");
        iterate_over(m, "Iterate over unordered_map");
        map_sanity(v);
    }
    /*/
