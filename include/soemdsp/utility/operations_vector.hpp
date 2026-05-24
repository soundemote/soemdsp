#pragma once

#include <chrono>
#include <functional>
#include <random>
#include <soemdsp/sehelper.hpp>

namespace soemdsp::memop {

//Inserts random values into a container from 0.0 to 1.0 inclusive
template<typename Container> void insertRandomNormalizedValues(Container& arr, size_t startIndex, size_t endIndex, std::function<double()> randomFunction) {
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = randomFunction();
    }

    auto minmaxPair  = std::minmax_element(arr.begin() + startIndex, arr.begin() + endIndex);
    double randomMin = *minmaxPair.first;
    double randomMax = *minmaxPair.second;

    double scale{};
    double offset{ 1.0 };

    if (randomMax != randomMin) {
        scale  = 1.0 / (randomMax - randomMin);
        offset = -randomMin * scale;
    }
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = *it * scale + offset;
    }
}
//Inserts random values into a container from minValue to maxValue inclusive
template<typename Container> void insertRandomValues(Container& arr, size_t startIndex, size_t endIndex, double minValue, double maxValue, std::function<double()> randomFunction) {
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = randomFunction();
    }

    auto minmaxPair  = std::minmax_element(arr.begin() + startIndex, arr.begin() + endIndex);
    double randomMin = *minmaxPair.first;
    double randomMax = *minmaxPair.second;

    double scale{};
    double offset{ maxValue };

    if (randomMax != randomMin) { //if range is not degenerate
        scale  = (maxValue - minValue) / (randomMax - randomMin);
        offset = minValue - randomMin * scale;
    }
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = *it * scale + offset;
    }
}

//Get a default_random_engine for use in std::algorithm that require a randomizer object using system time as a seed
inline std::default_random_engine getRandomEngine() {
    return std::default_random_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
}

//Finds and returns pointer of nearest element in std::vector. Higher element is returned if two are valid. Requires operators: - < <=
//Returns an iterator to the nearest element in the range [begin, end)
template<typename ForwardIt, typename T>
ForwardIt nearestElement(ForwardIt begin, ForwardIt end, const T& element) {
    if (begin == end) {
        return end;
    }

    const auto it = std::lower_bound(begin, end, element);

    //If the element is smaller than or equal to the first element
    if (it == begin) {
        return begin;
    }

    //If the element is larger than or equal to the last element
    if (it == end) {
        return std::prev(end);
    }

    //Compare distance to the element found and the one before it
    const auto prev_it = std::prev(it);

    //We use std::abs to compare the distance to the target
    if (std::abs(*it - element) <= std::abs(*prev_it - element)) {
        return it;
    }

    return prev_it;
}

template<typename t>
size_t nearestIndexOf(const t& target, const std::vector<t>& vecToSearch) {
    debug::CHECK(!vecToSearch.empty());

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

//returns 0 if no element found as well if element found is at start
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
void insert(std::vector<t>& v, size_t index, t element) {
    v.insert(v.begin() + index, element);
}

template<typename t>
void move(std::vector<t>& v, size_t oldIndex, size_t newIndex) {
    if (oldIndex > newIndex) {
        std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
    } else {
        std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
    }
}

template<typename t>
void erase(std::vector<t>& v, size_t index) {
    v.erase(v.begin() + index);
}

template<typename t>
void eraseIf(std::vector<t>& v, std::function<bool(const t&)> f /* = [](const t& obj) { return true; }*/) {
    v.erase(std::remove_if(v.begin(), v.end(), f), v.end());
}

template<typename t>
void eraseObject(std::vector<t>& v, t objectToRemove) {
    auto it = std::find(v.begin(), v.end(), objectToRemove);
    if (it != v.end()) {
        v.erase(it);
    }
}

template<typename t>
t popFront(std::vector<t>& v) {
    auto ret = std::move(v[0]);
    v.erase(v.begin());
    return std::move(ret);
}

//Appends a onto the end of b
template<typename t>
void append(std::vector<t>& a, const std::vector<t>& b) {
    a.insert(a.end(), b.begin(), b.end());
}

template<class t>
bool contains(const std::vector<t>& v, t elementToCheckFor) {
    return std::any_of(v.begin(), v.end(), [&](t e) { return e == elementToCheckFor; });
}

//Only pushes if element does not already exist.
template<class t>
void pushUnique(std::vector<t>& v, t newElement) {
    if (!contains(v, newElement)) {
        v.push_back(newElement);
    }
}

template<class t>
void sort(
  std::vector<t>& v, std::function<bool(const t&, const t&)> f = [](const t& a, const t& b) { return a < b; }) {
    std::stable_sort(v.begin(), v.end(), f);
}

//Returns a new std::vector of random elements from given std::vector attempting not to repeat indexes if num elements is larger than size.
template<class t>
std::vector<t> sample(const std::vector<t> v, int elements) {
    std::vector<t> ret;
    ret.reserve(elements);

    while (ret.size() < elements) {
        std::shuffle(v.begin(), v.end(), getRandomEngine());
        ret.insert(ret.end(), v.begin(), v.begin() + std::min(v.size(), elements - ret.size()));
    }

    return std::move(ret);
}

//Shuffles the given std::vector in place.
template<class t>
void shuffle(std::vector<t>& v) {
    std::shuffle(v.begin(), v.end(), getRandomEngine());
}

inline std::string tostring(const std::vector<std::string>& vec, char separator = '\0', size_t startIndex = 0, size_t numIndexes = 0) {
    if (vec.empty() || startIndex >= vec.size()) {
        return {}; //No elements to join or start index out of range
    }
    if (numIndexes == 0) {
        numIndexes = vec.size();
    }

    std::string s;
    size_t totalSize{};
    size_t endIndex = std::min(startIndex + numIndexes, vec.size());
    for (size_t i = startIndex; i < endIndex; ++i) {
        totalSize += vec[i].length() + 1; //extra for seperator
    }
    s.reserve(totalSize);

    s += vec[startIndex++];
    while (startIndex < endIndex) {
        s += separator + vec[startIndex++];
    }
    return std::move(s);
}
} //namespace soemdsp::memop
