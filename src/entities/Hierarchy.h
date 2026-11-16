#pragma once

#include "Entity.h"

#include <unordered_map>
#include <vector>

class Hierarchy {
public:
    void add(int e, int parent = NULL_ENTITY);
    bool setParent(int child, int newParent);
    void remove(int e);

    int getParent(int e) const;
    bool has(int e) const;
    bool isRoot(int e) const { return getParent(e) == NULL_ENTITY; }

    // children of one entity
    template <class F>
    void forEachChild(int e, F fn) const {
        auto it = children.find(e);
        if (it == children.end()) return;
        for (int c : it->second) fn(c);
    }

    // every entity
    template <class F>
    void forEachEntity(F fn) const {
        for (const auto& [e, p] : parents) fn(e);
    }

    template <class F>
    void forEachParent(F fn) const {
        for (const auto& [e, p] : parents)
            if (p == NULL_ENTITY) fn(e);
    }

private:
    bool isAncestor(int a, int b) const; // also true for a == b

    std::unordered_map<int, int> parents;
    std::unordered_map<int, std::vector<int>> children;
};