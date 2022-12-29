#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

template <typename I> struct find_result {
    find_result(I where, bool found) : where(where), found(found) {}
    I where;
    bool found = false;
};

template <typename VECITER> struct InsertResult {
    InsertResult(VECITER& where, bool didInsert = false)
        : inserted(didInsert), where(where) {}
    bool Inserted() const noexcept { return inserted; }
    VECITER Where() const noexcept { return where; }

    private:
    bool inserted = false;
    VECITER where;
};

struct UniqueKeysPolicy {
    template <typename VEC, typename K> static auto insert(VEC& v, const K& k) {
        auto it = std::lower_bound(v.begin(), v.end(), k);
        if (it == v.end()) {
            it = v.insert(it, k);
            return InsertResult{it, true};
        }
        return InsertResult{it, false};
    }
};

struct MultiKeysPolicy {
    template <typename VEC, typename K> static auto insert(VEC& v, const K& k) {
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

template <typename K, typename V> class SteveMap {
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

    auto size() const noexcept { return values.size(); }

    auto find_multiple(const K& key) {
        auto pr = std::make_pair(key, key);
        auto retval = std::equal_range(keys.real_begin(), keys.real_end(), pr);
        return retval;
    }

    auto find(const K& key) {
        auto it = std::lower_bound(keys.begin(), keys.end(), key,
            [](const auto& a, const auto& b) { return a.first == b; });
        return it;
    }

    void insert_range(const std::vector<V>& vals) {
        values.reserve(values.size() + vals.size());
        values.insert(values.end(), vals.begin(), vals.end());
        index_all();
    }

    void index_all() {
        // std::string s{"indexing all "};
        //  s += std::to_string(values.size());
        //  my::stopwatch sw(s);
        keys.clear();
        keys.resize(values.size());

        size_t idx = 0;
        for (auto&& v : values) {
            keys[idx] = std::make_pair(idx, v);
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
