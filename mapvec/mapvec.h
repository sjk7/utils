#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits> // false_type
#include <utility> // std::pair

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
template <typename> struct is_pair : std::false_type {};

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type {};

struct MultiKeysPolicy {
    template <typename VEC, typename K> static auto insert(VEC& v, const K& k) {
        auto it = std::lower_bound(v.begin(), v.end(), k);
        it = v.insert(it, k);
        return InsertResult{it, true};
    }
};

struct UniqueKeysPolicy {

    template <typename VEC, typename K>
    // returns the same as unordered_map would
    static inline auto insert(VEC& v, K&& k) {
        auto it = find(v, k);

        if (it == v.end()) { // ie if it is not ALREADY in the vector
            // pair<iterator,bool>
            auto rv = v.insert(it, k);
            return rv;
        }

        if constexpr (is_pair<typename VEC::value_type>::value) {
            if (it->first != k.first) {
                auto rv = v.insert(it, k);
                // auto ret = std::pair{rv, true};
                return rv;
            }
        } else {

            if (*it != k) {
                auto rv = v.insert(it, k);
                auto ret = std::make_pair(rv, true);
                return it;
            } else {
                return it;
            }
        }

        return v.end();
    }
    template <typename VEC, typename K> static auto find(VEC& v, const K& k) {

        if constexpr (is_pair<typename VEC::value_type>::value) {

            auto it = std::lower_bound(
                v.begin(), v.end(), k, [](const auto& a, const auto& b) {
                    bool ret = a.first < b.first;
                    return ret;
                });
            return it;

        } else {
            auto it = std::lower_bound(
                v.begin(), v.end(), k, [](const auto& a, const auto& b) {
                    bool ret = a < b;
                    return ret;
                });
            return it;
        }
    }
};

template <typename T, typename DUPES_POLICY = UniqueKeysPolicy>
class SortedVector : private std::vector<T> {
    using base = std::vector<T>;
    DUPES_POLICY dupesPolicy = DUPES_POLICY{};

    public:
    using cit = typename base::const_iterator;
    using it = typename base::iterator;
    using base::operator[];

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
        auto where = dupesPolicy.insert(b, std::forward<T>(val));
        return where;
    }

    auto find(const T& val) const noexcept {
        return dupesPolicy.find(*this, val);
    }
    auto find(const T& val) noexcept { return dupesPolicy.find(*this, val); }

    auto push_back(const T& t) { return base::push_back(t); }
};

template <typename K, typename V> class SteveMap {
    private:
    using internal_pair_type = std::pair<K, size_t>; //
    SortedVector<internal_pair_type>
        m_keys; // find by key, the second member of the pair is the index
                // of the value in values
    std::vector<V> m_values;

    public:
    using pair_type = std::pair<K, V>;
    using values_type = std::vector<V>;
    using iterator_type = typename values_type::iterator;
    using insert_return_type = std::pair<iterator_type, bool>;
    const SortedVector<internal_pair_type>& keys() const { return m_keys; }
    const std::vector<V>& values() const { return m_values; }

    auto size() const noexcept { return m_values.size(); }

    auto find_multiple(const K& key) {
        auto pr = std::make_pair(key, key);
        auto retval
            = std::equal_range(m_keys.real_begin(), m_keys.real_end(), pr);
        return retval;
    }

    auto find(const K& key) {
        auto it = m_keys.find({key, 0});
        if (it != m_keys.end() && it->first == key) {
            return it;
        }
        return m_keys.end();
    }

    void insert_range(const std::vector<pair_type>& key_value_pairs) {
        const auto old_size = m_values.size();
        m_values.reserve(m_values.size() + key_value_pairs.size());
        m_keys.reserve(m_keys.size() + key_value_pairs.size());
        for (const auto& v : key_value_pairs) {
            insert(v);
        }
    }

    /*/
    void index_all() {
        // std::string s{"indexing all "};
        //  s += std::to_string(values.size());
        //  my::stopwatch sw(s);
        m_keys.clear();
        m_keys.resize(m_values.size());

        size_t idx = 0;
        for (auto&& v : m_values) {
            auto pr = std::pair{idx, v};
            m_keys[idx] = pr;
            ++idx;
        }
    }
    /*/

    auto insert(const pair_type& pr) noexcept {
        const auto pos = m_values.size();
        auto insert_result = m_keys.insert({pr.first, pos});
        if (insert_result != m_keys.end()) {
            m_values.push_back(pr.second);
        }
        assert(m_keys.size() == m_values.size());
        return insert_result;
    }

    void reserve(size_t sz) {
        m_keys.reserve(sz);
        m_values.reserve(sz);
    }

    auto begin() const noexcept { return m_keys.begin(); }
    auto end() const noexcept { return m_keys.end(); }
};
