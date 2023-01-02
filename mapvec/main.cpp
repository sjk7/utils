// This is an independent project of an individual developer. Dear PVS-Studio,
//  please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#include "../../utils/my_timing.hpp"
#include "../../utils/my_utils.hpp"
#include "mapvec.h"
#include <random>
#include <unordered_map>
#include <algorithm>
#include <numeric>


using namespace std;

using intspair_t = std::pair<int, string>;
std::vector<intspair_t> pairs;

template <typename M, typename KEYSTOFIND>
size_t find_keys(const M& m, const KEYSTOFIND& keys, string_view what) {

    my::stopwatch sw(what);
    volatile size_t ffs = 0;
    for (const auto& key : keys) {
        auto found = m.find(key.first);
        if (found != m.end()) ffs = ffs + 1;
    }
    return ffs;
}

template <typename M, typename V>
unsigned long add_to_map(M& m, V& pairs, string id) {

    volatile int ctr = 0;
    my::stopwatch sw(id);

    m.reserve(pairs.size());
    for (const auto& pr : pairs) {
        auto ins = m.insert(pr);
        assert(ins.second);
        ctr = ctr + 1;
    }

    cout << "fake: " << ctr << endl;
    return ctr;
}

template <typename MAPVEC, typename VECITEMS>
unsigned long insert_batch(MAPVEC& m, const VECITEMS& v) {

    volatile unsigned long u = 0;
    auto s = my::utils::strings::concat(
        "Inserting ", v.size(), " items into mapvec using batch inserter");
    my::stopwatch sw(s);
    {
        my::batch_inserter inserter(m, v.size());

        for (const auto& pr : v) {
            inserter.add(pr);
            u = u + 1;
        }
    }

    return u;
}

template <typename RET, typename PAIRS>
auto construct_map(RET&, const PAIRS& pairs, const string& id) {
    my::stopwatch sw(id);
    return RET(pairs.begin(), pairs.end());
}

template <typename T>
unsigned long iterate_over(const T& m, const std::string& what) {

    volatile unsigned long fake = 0;
    my::stopwatch sw(what);
    for (const auto thing : m) {
        fake = fake + thing.first;
    }
    cout << "fake: " << fake << endl;
    return fake;
}

using stringvec_t = std::vector<std::pair<std::string, std::string>>;
stringvec_t create_string_pairs(const size_t N) {

    stringvec_t ret;
    ret.reserve(N);
    for (size_t i = 0; i < N; i++) {
        std::string s = std::to_string(i);
        ret.emplace_back(std::pair{s, s});
    }
    return ret;
}

template <typename T>
unsigned long add_to_map(T& m, const stringvec_t& v, const std::string& what) {
    my::stopwatch sw(what);
    volatile unsigned long x = 0;
    for (const auto& pr : v) {
        auto inserted = m.insert(pr);
        assert(inserted.second);
        if (inserted.second) x = x + 1;
    }

    return x;
}

template <typename T>
T create_map_from_strings(T& m, const stringvec_t& v, const std::string& what) {
    my::stopwatch sw(what);
    return T(v.begin(), v.end());
}

void test_using_strings(const size_t N) {

    my::vector_map<string, string> vm;
    std::unordered_map<string, string> um;
    //Loki::AssocVector<string, string> am;
    auto pairs = create_string_pairs(N);
    cout << "Testing with " << N << " string pairs" << endl;

    vm = create_map_from_strings(vm, pairs, "Creating vector_map from range");
    um = create_map_from_strings(
        um, pairs, "Creating unordered_map from range");

    //am = create_map_from_strings(am, pairs, "Creating Loki from range");
    assert(am.size() == um.size());
    assert(vm.size() == um.size());
    vm.clear();
    um.clear();

    volatile unsigned long a
        = add_to_map(um, pairs, "Adding string pairs to unordered_map");
    // volatile unsigned long b
    //      = add_to_map(vm, pairs, "Adding string pairs to vector_map");

    volatile unsigned long x = insert_batch(vm, pairs);
    assert(x == a);

    auto rng = std::default_random_engine{};
    auto& shuffled_keys = pairs;
    std::shuffle(std::begin(pairs), std::end(pairs), rng);

    volatile auto meh = find_keys(
        um, shuffled_keys, "Finding findable strings in unordered_map");
    volatile auto moo
        = find_keys(vm, shuffled_keys, "Finding findable strings in vec_map");
   // volatile auto loki
    //    = find_keys(am, shuffled_keys, "Finding findable strings in Loki");
    assert(meh == moo);
    //assert(loki == moo);

    for (auto& pr : shuffled_keys) {
        pr.first += "x";
    }

    meh = find_keys(
        um, shuffled_keys, "Finding unfindable strings in unordered_map");
    moo = find_keys(vm, shuffled_keys, "Finding unfindable strings in vec_map");
    //loki = find_keys(am, shuffled_keys, "Finding unfindable strings in Loki");
    assert(meh == moo);
    assert(meh == 0);
    //assert(loki == 0);
}

void make_data(const size_t N) {

    test_using_strings(N);

    cout << "Now using <int, string>" << endl << endl;
    std::vector<int> v(N);
    std::iota(std::begin(v), std::end(v), 0);
    pairs.reserve(N);
    for (auto i : v) {
        pairs.emplace_back(i, std::to_string(i));
    }

    my::vector_map<int, string> mv;
    unordered_map<int, string> um;
   
    volatile auto a = add_to_map(um, pairs, "Adding to unordered_map");
    volatile auto b = add_to_map(mv, pairs, "Adding to vector_map");

    assert(a == b);
    assert(b == c);
    cout << a << ":" << b <<  endl;

    assert(um.size() == mv.size());
    assert(um.find(77)->second == "77");
    assert(mv.find(77)->second == "77");


    a = iterate_over(um, "Iterating over unordered_map");
    b = iterate_over(mv, "Iterating over vector_map");
   
    assert(a == b);
 
    cout << a << ":" << b << ":"  << endl;

    my::vector_map<int, string> mva;
    unordered_map<int, string> uma;
    //Loki::AssocVector<int, string> ama;

    uma = construct_map(uma, pairs, "Creating unordered_map from range");
    cout << mv.size() << endl;
    mva = construct_map(mva, pairs, "Creating vector_map from range");
    cout << mv.size() << endl;
    assert(mva.size() == uma.size());
   // ama = construct_map(ama, pairs, "Creating loki from range");

    std::vector<intspair_t> with_dupes(pairs.begin(), pairs.end());
    with_dupes.insert(with_dupes.end(), pairs.begin(), pairs.end());
    assert(with_dupes.size() == pairs.size() * 2);

    auto rng = std::default_random_engine{};
    auto& shuffled_keys = pairs;
    std::shuffle(std::begin(pairs), std::end(pairs), rng);

    volatile auto meh = find_keys(
        uma, shuffled_keys, "Finding findable ints in unordered_map");
    volatile auto moo
        = find_keys(mva, shuffled_keys, "Finding findable ints in vec_map");
   // volatile auto Loki
   //     = find_keys(ama, shuffled_keys, "Finding findable ints in Loki");
    assert(meh == moo);
    //assert(Loki == moo);

    for (auto& pr : shuffled_keys) {
        pr.first += 1000000000;
    }

    meh = find_keys(
        uma, shuffled_keys, "Finding unfindable ints in unordered_map");
    moo = find_keys(mva, shuffled_keys, "Finding unfindable ints in vec_map");
    //Loki = find_keys(ama, shuffled_keys, "Finding unfindable ints in Loki");
    assert(meh == moo);
    assert(meh == 0);
    //assert(Loki == moo);
}

template <typename M>
void insert_into_with_checks(M& m, std::pair<int, string> what,
    size_t expected_size, bool expected_exist = false) {

    auto pr = m.insert(what);
    if (!expected_exist) {

        assert(pr.second);
        assert(pr.first != m.end());
        assert((*pr.first).second == what.second);
        assert((*pr.first).first == what.first);
    } else {
        assert(!pr.second);
        assert(pr.first == m.find(what.first));
    }

    assert(m.size() == expected_size);
    assert(!m.empty());
}

int main() {

    my::vector_map<int, string> vm;
    unordered_map<int, string> um;

    assert(vm.empty());

    insert_into_with_checks(vm, {1, "one"}, 1);
    insert_into_with_checks(um, {1, "one"}, 1);

    insert_into_with_checks(um, {1, "one"}, 1, true);
    insert_into_with_checks(vm, {1, "one"}, 1, true);

    auto pr = um.find(1);
    assert(pr->first == 1);

    auto p = vm.find(1);
    assert(p->first == 1);

    // test copy assignment
    my::vector_map<int, string> v;
    v = vm;

    // test copy construction
    my::vector_map<int, string> z = vm;

    assert(v.size() == vm.size() && z.size() == vm.size());
    z.insert({2, "two"});
    assert(z.size() == v.size() + 1);

    make_data(1000000);

    cout << endl
         << endl
         << "Conclude that vector_map is better than unordered_map, but only "
            "if the keys are NOT strings!!"
         << endl
         << "Basically any type that is cheap to move is OK for the key, else "
            "just use unordered_map : it is the insertions for strings that "
            "kills it."
         << endl
         << "If you can create vector_map with a range rather than individual "
            "insert()s, then strings might be OK"
         << endl;

    return 0;
}
