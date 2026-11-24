#pragma once

#include <unordered_map>
#include <glm/glm.hpp>
#include "renderer/transform.h"

template <class T>
class ComponentStore {
public:
    using Map = std::unordered_map<int, T>;

    T& add(int e, T value = {}) { return data[e] = std::move(value); }
    void remove(int e) { data.erase(e); }
    bool has(int e) const { return data.contains(e); }
    T* get(int e) { auto it = data.find(e); return it == data.end() ? nullptr : &it->second; }

    template <class F>
    void each(F fn) { for (auto& [e, c] : data) fn(e, c); }

    typename Map::iterator       begin() { return data.begin(); }
    typename Map::iterator       end() { return data.end(); }
    typename Map::const_iterator begin() const { return data.begin(); }
    typename Map::const_iterator end()   const { return data.end(); }

private:
    Map data;
};