#include "Hierarchy.h"

#include <algorithm>

void Hierarchy::add(int e, int parent) {
    parents[e] = NULL_ENTITY;
    children[e] = {};
    if (parent != NULL_ENTITY) setParent(e, parent);
}

bool Hierarchy::setParent(int child, int newParent) {
    if (!has(child)) return false;
    if (newParent != NULL_ENTITY && !has(newParent)) return false;
    if (newParent != NULL_ENTITY && isAncestor(child, newParent)) return false;

    int old = parents[child];
    if (old != NULL_ENTITY) {
        auto& sib = children[old];
        sib.erase(std::remove(sib.begin(), sib.end(), child), sib.end());
    }

    parents[child] = newParent;
    if (newParent != NULL_ENTITY) children[newParent].push_back(child);
    return true;
}

void Hierarchy::remove(int e) {
    if (!has(e)) return;

    for (int c : children[e]) parents[c] = NULL_ENTITY;
    setParent(e, NULL_ENTITY);
    parents.erase(e);
    children.erase(e);
}

int Hierarchy::getParent(int e) const {
    auto it = parents.find(e);
    return it != parents.end() ? it->second : NULL_ENTITY;
}

bool Hierarchy::has(int e) const {
    return parents.count(e) > 0;
}

bool Hierarchy::isAncestor(int a, int b) const {
    for (int cur = b; cur != NULL_ENTITY; cur = parents.at(cur))
        if (cur == a) return true;
    return false;
}