#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#include "../utils/my_timing.hpp"
#pragma GCC diagnostic ignored "-Wvolatile"

template <typename I>
struct find_result {
  find_result(I where, bool found) : where(where), found(found) {}
  I where;
  bool found = false;
};

template <typename VECITER>
struct InsertResult {
  InsertResult(VECITER& where, bool didInsert = false)
      : inserted(didInsert), where(where) {}
  bool Inserted() const noexcept { return inserted; }
  VECITER Where() const noexcept { return where; }

 private:
  bool inserted = false;
  VECITER where;
};

struct UniqueKeysPolicy {
  template <typename VEC, typename K>
  static auto insert(VEC& v, const K& k) {
    auto it = std::lower_bound(v.begin(), v.end(), k);
    if (it == v.end()) {
      it = v.insert(it, k);
      return InsertResult{it, true};
    }
    return InsertResult{it, false};
  }
};

struct MultiKeysPolicy {
  template <typename VEC, typename K>
  static auto insert(VEC& v, const K& k) {
    auto it = std::lower_bound(v.begin(), v.end(), k);
    it = v.insert(it, k);
    return InsertResult{it, true};
  }
};

template <typename T, typename DUPES_POLICY = UniqueKeysPolicy>
class SortedVector : private std::vector<T> {
  using base = std::vector<T>;
  DUPES_POLICY dupesPolicy = DUPES_POLICY{};

 public:
  using cit = typename base::const_iterator;
  using it = typename base::iterator;

  using base::cbegin;
  using base::cend;
  using base::clear;
  using base::data;

  using base::reserve;
  using base::resize;
  using base::size;
  cit begin() const noexcept { return base::cbegin(); }

  cit end() const noexcept { return base::cend(); }
  auto begin() { return base::begin(); }
  auto end() { return base::end(); }

  auto insert(const T& val) {
    // auto where = std::lower_bound(base::begin(), base::end(), val);
    // auto it = base::insert(where, val);
    base& b = *this;
    auto where = dupesPolicy.insert(b, val);
    return where;
  }

  auto insert(T&& val) {
    base& b = *this;
    auto where = dupesPolicy.insert(b, val);
    return where;
  }

  cit find(const T& val) {
    auto where = std::lower_bound(base::cbegin(), base::cend(), val);
    bool found = where != base::cend();
    if (found) {
      const auto& v = *where;
      if (v == val) {
        found = true;
      } else {
        found = false;
        return base::cend();
      }
    }
    return where;
  }

  auto push_back(const T& t) { return base::push_back(t); }
  auto& operator[](int i) noexcept { return base::operator[](i); }
};

template <typename K, typename V>
class SteveMap {
 private:
  using internal_pair_type = std::pair<K, V>;
  SortedVector<internal_pair_type> keys;
  std::vector<V> values;

 public:
  using pair_type = std::pair<K, V>;
  using values_type = std::vector<V>;
  using keys_type = SortedVector<K>;
  using iterator_type = typename values_type::iterator;
  using insert_return_type = std::pair<iterator_type, bool>;
  using find_return_t = typename std::unordered_map<K, V>::const_iterator;
  using find_return_type = find_return_t*;

  auto size() const noexcept { return values.size(); }

  auto find_multiple(const K& key) {
    auto pr = std::make_pair(key, key);
    auto retval = std::equal_range(keys.real_begin(), keys.real_end(), pr);
    return retval;
  }

  auto find(const K& key) {
    static constexpr K empty_value = K();
    auto it = std::lower_bound(
        keys.begin(), keys.end(), key,
        [](const auto& a, const auto& b) { return a.first == b; });
    if (it == keys.end()) {
      return std::pair{it, empty_value};
    }
    auto idx = it->second;
    auto pair_result = std::pair{it, values[idx]};
    return pair_result;
  }

  void insert_range(const std::vector<V>& vals) {
    values.reserve(values.size() + vals.size());
    values.insert(values.end(), vals.begin(), vals.end());
    index_all();
  }

  void index_all() {
    std::string s{"indexing all "};
    s += std::to_string(values.size());
    my::stopwatch sw(s);
    keys.clear();
    keys.resize(values.size());

    size_t idx = 0;
    for (auto&& v : values) {
      keys[idx] = {idx, v};
      ++idx;
    }
  }

  void insert(const pair_type& pr) noexcept {
    const auto pos = values.size();
    keys.insert({pr.first, pos});
    values.push_back(pr.second);
  }

  void reserve(size_t sz) {
    keys.reserve(sz);
    values.reserve(sz);
  }

  auto begin() const noexcept { return keys.begin(); }
  auto end() const noexcept { return keys.end(); }
};

using namespace std;

void test_as_keys(size_t);
int main() {
  cout << "Size of float: " << sizeof(float) << endl;
  cout << "Size of double: " << sizeof(double) << endl;
  SortedVector<int> sv;
  auto insertResult = sv.insert(11);
  assert(insertResult.Where() != sv.end());
  assert(insertResult.Inserted());
  assert(sv.size() == 1);
  assert(sv.find(11) != sv.end());
  assert(*(sv.find(11)) == 11);
  auto ins = sv.insert(11);
  assert(ins.Inserted() == false);
  assert(*ins.Where() == 11);
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
/*/
my::stopwatch sw("timing", false);
SortedVector<int> sv;
sv.insert(3);
sv.insert(0);
sv.insert(5);
sv.insert(100);
sv.insert(70);
sv.insert(1);
assert(sv.size() == 6);

auto f = sv.find(0);
assert(f != sv.end());
assert(f == sv.cbegin());
assert((*f) == 0);

f = sv.find(100);
auto e = sv.end();
assert(f != e);

f = sv.find(99);
assert(f == sv.cend());

std::cout << sw.elapsed_ns() << std::endl;
/*/
#ifdef NDEBUG
  static const int N = 20'000'000;
#else
  static const int N = 1'000'000;
  test_as_keys(20'000);
#endif

  test_as_keys(N);
}

template <typename T>
void add_stuff(T& t, std::string what, const int N) {
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
      rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());
  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, N - 1);
  std::vector<V> vals;
  vals.reserve(N);

  for (int i = 0; i < N; i++) {
    const auto r = i;  // distrib(gen);
    vals.push_back(r);
  }

  t.insert_range(vals);
}

template <typename K, typename V>
void test_find(std::unordered_map<K, V>& m, SteveMap<K, V>& sm) {
  const int N = m.size();
  std::random_device
      rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());
  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, m.size() - 1);
  volatile unsigned int fake_total = 0;

  {
    my::stopwatch sw("Finding randomly in " + std::to_string(N) +
                     " unordered_map items");
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
    my::stopwatch sw("Finding randomly in " + std::to_string(N) +
                     " SteveMap items");
    for (int i = 0; i < N; i++) {
      int r = distrib(gen);
      auto pr = sm.find(r);
      auto v = pr.second;
      fake_total += v;
    }
  }
  // cout << fake_total << '\r';
}

template <typename T>
void iterate_over(const T& t, const char* what) {
  volatile unsigned long fake = 0;
  my::stopwatch sw(what);
  for (const auto& pr : t) {
    fake += pr.second;
  }
  // cout << "iterator fake is: " << '\r';
  //;
}

template <typename T>
void map_sanity(const T& t) {
  volatile unsigned long fake = 0;

  for (const auto& pr : t) {
    bool b = pr.first == pr.second;
    assert(b);
    fake += b;
  }

  // std::cout << fake << '\r';
  //;
}

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
