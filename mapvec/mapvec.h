// This is an independent project of an individual developer. Dear PVS-Studio,
//  please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits> // false_type
#include <utility> // std::pair
#include <iterator>
#include <deque>

namespace my {

template <typename I> struct inserted_return_type {
    I where;
    bool inserted;
    inserted_return_type(I where, bool inserted)
        : where(where), inserted(inserted) {}
};

template <typename M> struct batch_inserter {

    private:
    std::vector<typename M::value_type> items;

    public:
    batch_inserter(M& m, size_t size_hint = 1) : map(m) {
        items.reserve(size_hint);
    }

    M& map;

    ~batch_inserter() {
        map.insert_default(map.end(), items.begin(), items.end());
        map.sort();
    }

    template <typename P> size_t add(P&& pair) {
        items.emplace_back(std::forward<P>(pair));
        return items.size();
    }
};

template <typename T> using basis = std::vector<T>;
template <class T, class Compare = std::less<T>, class UNIQUE = std::true_type>
struct sorted_vector : protected basis<T> {

    private:
    Compare cmp;
    template <typename X> friend struct batch_inserter;

    public:
    using base = basis<T>;
    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    using insert_return_type = inserted_return_type<iterator>;

    using base::begin;
    using base::clear;
    using base::empty;
    using base::end;
    using base::reserve;
    using base::size;

    sorted_vector(const Compare& c = Compare()) : base(), cmp(c) {}

    template <class InputIterator>
    sorted_vector(
        InputIterator first, InputIterator last, const Compare& c = Compare())
        : base(first, last), cmp(c) {
        sort();
    }

    virtual ~sorted_vector() = default;

    void sort() {
        std::sort(begin(), end(), cmp);
        if constexpr (UNIQUE::value)
            base::erase(std::unique(base::begin(), base::end()), base::end());
    }
    insert_return_type insert(const T& t) {
        iterator i = std::lower_bound(begin(), end(), t, cmp);
        if (i == end() || cmp(t, *i)) {
            i = base::insert(i, t);
            return insert_return_type{i, true};
        }
        return insert_return_type{i, false};
    }

    const_iterator find(const T& t) const {
        const_iterator i = std::lower_bound(base::begin(), base::end(), t, cmp);

        return i == end() || cmp(t, *i) ? end() : i;
    }

    iterator find(const T& t) {
        iterator i = std::lower_bound(base::begin(), base::end(), t, cmp);

        return i == end() || cmp(t, *i) ? end() : i;
    }

    private:
    template <typename ForwardIterator>
    auto insert_default(
        typename base::iterator ita, ForwardIterator itb, ForwardIterator itc) {
        return base::insert(ita, itb, itc);
    }
};

template <typename K, typename V> struct key_value_compare {
    bool operator()(const std::pair<K, V>& a, const std::pair<K, V>& b) const {
        return a.first < b.first;
    }
};

template <typename K, typename V>
using key_value_pair_type = typename std::pair<K, V>;

/*/
/////////////// REGARDING COMPARISON AGAINST UNORDERED_MAP /////////////////
 ////////////// NOTE: this class is slow with string keys //////////////////
 ////////////// and across the board slower than unordered_map on MAC //////
 ////////////// In Linux, you should test. But again, avoid string keys ////
 ///////////////////////////////////////////////////////////////////////////
/*/
template <class K, class V, class Compare = key_value_compare<K, V>>
struct vector_map
    : sorted_vector<key_value_pair_type<K, V>, key_value_compare<K, V>> {

    using value_type = key_value_pair_type<K, V>;
    using base_type
        = sorted_vector<key_value_pair_type<K, V>, key_value_compare<K, V>>;

    using key_type = K;
    using val_type = V;
    key_value_compare<K, V> cmp = key_value_compare<K, V>();
    using const_iterator = typename base_type::const_iterator;
    using iterator = typename base_type::iterator;

    using insert_ret_t = std::pair<iterator, bool>;

    template <typename InputIterator>
    vector_map(InputIterator first, InputIterator last)
        : base_type(first, last, cmp) {}

    vector_map() : base_type(cmp) {}
    virtual ~vector_map() = default;

    auto insert(const value_type& keyval) {

        auto irt = base_type::insert(keyval);
        if (irt.inserted) {
            return std::pair{irt.where, true};
        }
        return std::pair{irt.where, false};
    }

    // to emulate unordered_map, we need to be able to find ONLY by key:
    auto find(const K& key) {
        std::pair f{key, V()};
        auto found = base_type::find(f);
        return found;
    }
    auto find(const K& key) const {
        std::pair f{key, V()};
        auto found = base_type::find(f);
        return found;
    }

    void reserve(size_t N) { base_type::reserve(N); }
    void clear() { base_type::clear(); }
};

} // namespace my
