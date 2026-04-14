#pragma once

#include <chrono>
#include <random>
#include <functional>

namespace soemdsp::VEC {
using std::vector;
using std::string;

// Get a default_random_engine for use in std::algorithm that require a randomizer object using system time as a seed
inline std::default_random_engine getRandomEngine() {
    return std::default_random_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
}

// Finds and returns pointer of nearest element in vector. Higher element is returned if two are valid. Requires operators: - < <=
template<class t>
t* nearestElement(const vector<t>& v, const t& element) {
    if (v.size() == 0)
        return {};

    const auto it = std::lower_bound(v.begin(), v.end(), element);

    if (it == v.begin())
        return v[0];

    if (abs(*it - element) <= abs(*(it - 1) - element))
        return it;

    return (it - 1);
}

template<typename t>
size_t nearestIndexOf(const t& target, const std::vector<t>& vecToSearch) {
    SE_ASSERT(!vecToSearch.empty());

    t minDiff = std::abs(target - vecToSearch[0]);

    size_t idx{};
    for (size_t i = 0; i < vecToSearch.size(); ++i) {
        double diff = std::abs(target - vecToSearch[i]);
        if (diff < minDiff) {
            minDiff = diff;
            idx     = i;
        }
    }

    return idx;
}

// returns 0 if no element found as well if element found is at start
template<typename t>
size_t indexOf(const t& target, const std::vector<t>& vecToSearch) {
    for (size_t i = 0; i < vecToSearch.size(); ++i) {
        if (vecToSearch[i] == target) {
            return i;
        }
    }
    return {};
}

template<typename t>
void insert(vector<t>& v, size_t index, t element) {
    v.insert(v.begin() + index, element);
}

template<typename t>
void move(vector<t>& v, size_t oldIndex, size_t newIndex) {
    if (oldIndex > newIndex)
        std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
    else
        std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
}

template<typename t>
void erase(vector<t>& v, size_t index) {
    v.erase(v.begin() + index);
}

template<typename t>
void eraseIf(vector<t>& v, std::function<bool(const t&)> f /* = [](const t& obj) { return true; }*/) {
    v.erase(std::remove_if(v.begin(), v.end(), f), v.end());
}

template<typename t>
void eraseObject(vector<t>& v, t objectToRemove) {
    auto it = std::find(v.begin(), v.end(), objectToRemove);
    if (it != v.end())
        v.erase(it);
}

template<typename t>
t popFront(vector<t>& v) {
    auto ret = std::move(v[0]);
    v.erase(v.begin());
    return std::move(ret);
}

// Appends a onto the end of b
template<typename t>
void append(vector<t>& a, const std::vector<t>& b) {
    a.insert(a.end(), b.begin(), b.end());
}

template<class t>
bool contains(const vector<t>& v, t elementToCheckFor) {
    return std::any_of(v.begin(), v.end(), [&](t e) { return e == elementToCheckFor; });
}

// Only pushes if element does not already exist.
template<class t>
void pushUnique(vector<t>& v, t newElement) {
    if (!contains(v, newElement))
        v.push_back(newElement);
}

template<class t>
void sort(
  vector<t>& v, std::function<bool(const t&, const t&)> f = [](const t& a, const t& b) { return a < b; }) {
    std::stable_sort(v.begin(), v.end(), f);
}

// Returns a new vector of random elements from given vector attempting not to repeat indexes if num elements is larger than size.
template<class t>
vector<t> sample(const vector<t> v, int elements) {
    vector<t> ret;
    ret.reserve(elements);

    while (ret.size() < elements) {
        std::shuffle(v.begin(), v.end(), getRandomEngine());
        ret.insert(ret.end(), v.begin(), v.begin() + std::min(v.size(), elements - ret.size()));
    }

    return std::move(ret);
}

// Shuffles the given vector in place.
template<class t>
void shuffle(vector<t>& v) {
    std::shuffle(v.begin(), v.end(), getRandomEngine());
}

inline string toString(const vector<string>& vec, char separator = '\0', size_t startIndex = 0, size_t numIndexes = 0) {
    if (vec.empty() || startIndex >= vec.size()) {
        return {}; // No elements to join or start index out of range
    }
    if (numIndexes == 0) {
        numIndexes = vec.size();
    }

    string s;
    size_t totalSize{};
    size_t endIndex = std::min(startIndex + numIndexes, vec.size());
    for (size_t i = startIndex; i < endIndex; ++i) {
        totalSize += vec[i].length() + 1; // extra for seperator
    }
    s.reserve(totalSize);

    s += vec[startIndex++];
    while (startIndex < endIndex) {
        s += separator + vec[startIndex++];
    }
    return std::move(s);
}
} // namespace soemdsp::VEC
