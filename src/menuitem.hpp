#ifndef MENUITEM_HPP
#define MENUITEM_HPP

#include <raylib.h>

enum class Action { Details, Default, Vertex, Edge, WeightedEdge, Search, BFS, DFS, Dijkstra };

struct MenuItem {
    Rectangle rect;
    Color color;
    Action action;
    bool visible = true;

    inline bool isVisible() const { return this->visible; }
};

#endif  // MENUITEM_HPP
