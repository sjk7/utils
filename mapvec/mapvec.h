#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits> // false_type
#include <utility> // std::pair
#include <iterator>

namespace my {

template <typename I> struct inserted_return_type {
    I where;
    bool inserted;
    inserted_return_type(I where, bool inserted)
        : where(where), inserted(inserted) {}
};
template <class T, class Compare = std::less<T>, class UNIQUE = std::true_type>
struct sorted_vector {
    private:
    // using std::lower_bound;
    // using std::vector;

    std::vector<T> V;

    public:
    void reserve(size_t N) { V.reserve(N); }
    size_t size() const noexcept { return V.size(); }
    bool empty() const noexcept { return V.empty(); }

    sorted_vector(const Compare& c = Compare()) : V(), cmp(c) {}
    template <class InputIterator>
    sorted_vector(
        InputIterator first, InputIterator last, const Compare& c = Compare())
        : V(first, last), cmp(c) {
        std::sort(begin(), end(), cmp);
        if constexpr (UNIQUE::value)
            V.erase(std::unique(V.begin(), V.end()), V.end());
    }
    virtual ~sorted_vector() = default;
    void clear() { V.clear(); }
    Compare cmp;
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    iterator begin() { return V.begin(); }
    iterator end() { return V.end(); }
    const_iterator begin() const { return V.begin(); }
    const_iterator end() const { return V.end(); }
    using insert_return_type = inserted_return_type<iterator>;

    insert_return_type insert(const T& t) {
        iterator i = std::lower_bound(begin(), end(), t, cmp);
        if (i == end() || cmp(t, *i)) {
            i = V.insert(i, t);
            return insert_return_type{i, true};
        }
        return insert_return_type{i, false};
    }

    const_iterator find(const T& t) const {
        const_iterator i = std::lower_bound(begin(), end(), t, cmp);
        return i == end() || cmp(t, *i) ? end() : i;
    }
};

template <typename K, typename V> struct key_value_compare {
    bool operator()(const auto& a, const auto& b) const {
        return a.first < b.first;
    }
};

template <typename K, typename V>
using key_value_pair_type = typename std::pair<K, V>;

/*/
////////////////////////////////////////////////////////////////////////////
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
    using const_iterator = base_type::const_iterator;
    using iterator = base_type::iterator;

    using insert_ret_t = std::pair<iterator, bool>;

    template <typename InputIterator>
    vector_map(InputIterator first, InputIterator last)
        : base_type(first, last, cmp) {}

    vector_map() : base_type(cmp) {}

    auto insert(value_type keyval) {

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

    void reserve(size_t N) { base_type::reserve(N); }
    void clear() { base_type::clear(); }
};
} // namespace my
