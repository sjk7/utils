
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

auto add_to_vector_map(std::vector<intspair_t> pairs, const string& id) {
    my::stopwatch sw(id);
    return my::vector_map<int, string>(pairs.begin(), pairs.end());
}

auto add_to_unordered_map(std::vector<intspair_t> pairs, const string& id) {
    my::stopwatch sw(id);
    return std::unordered_map<int, string>(pairs.begin(), pairs.end());
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
    auto pairs = create_string_pairs(N);
    cout << "Testing with " << N << " string pairs" << endl;

    vm = create_map_from_strings(vm, pairs, "Creating vector_map from range");
    um = create_map_from_strings(
        um, pairs, "Creating unordered_map from range");

    assert(vm.size() == um.size());
    vm.clear();
    um.clear();

    unsigned long a
        = add_to_map(um, pairs, "Adding string pairs to unordered_map");
    unsigned long b
        = add_to_map(vm, pairs, "Adding string pairs to vector_map");

    assert(a == b);
}

void make_data(const size_t N) {

    test_using_strings(N / 10);

    std::vector<int> v(N);
    std::iota(std::begin(v), std::end(v), 0);
    pairs.reserve(N);
    for (auto i : v) {
        pairs.emplace_back(i, std::to_string(i));
    }

    my::vector_map<int, string> mv;
    unordered_map<int, string> um;
    add_to_map(um, pairs, "Adding to unordered_map");
    add_to_map(mv, pairs, "Adding to vector_map");

    assert(um.size() == mv.size());
    assert(um.find(77)->second == "77");
    assert(mv.find(77)->second == "77");

    auto a = iterate_over(um, "Iterating over unordered_map");
    auto b = iterate_over(mv, "Iterating over vector_map");
    assert(a == b);

    my::vector_map<int, string> mva;
    unordered_map<int, string> uma;
    uma = add_to_unordered_map(pairs, "Creating unordered_map from range");
    cout << mv.size() << endl;
    mva = add_to_vector_map(pairs, "Creating vector_map from range");
    cout << mv.size() << endl;
    assert(mva.size() == uma.size());

    std::vector<intspair_t> with_dupes(pairs.begin(), pairs.end());
    with_dupes.insert(with_dupes.end(), pairs.begin(), pairs.end());
    assert(with_dupes.size() == pairs.size() * 2);

    uma = add_to_unordered_map(
        with_dupes, "Creating unordered_map from range (with dupes)");
    cout << mv.size() << endl;
    mva = add_to_vector_map(
        with_dupes, "Creating vector_map from range (with dupes)");
    cout << mv.size() << endl;
    assert(mva.size() == uma.size());
}

template <typename M>
void insert_into(M& m, std::pair<int, string> what, size_t expected_size,
    bool expected_exist = false) {

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

    insert_into(vm, {1, "one"}, 1);
    insert_into(um, {1, "one"}, 1);

    insert_into(um, {1, "one"}, 1, true);
    insert_into(vm, {1, "one"}, 1, true);

    auto pr = um.find(1);
    assert(pr->first == 1);

    auto p = vm.find(1);
    assert(p->first == 1);

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
