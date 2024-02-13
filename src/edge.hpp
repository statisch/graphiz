#ifndef EDGE_HPP
#define EDGE_HPP

#include <string>

struct Edge {
    static int instanceCounter;
    int id;
    int fromId;
    int toId;
    // weight is string to simplify the update process
    std::string weight;
    bool weighted = false;
    bool usable = true;

    Edge(int, int);
    Edge(int, int, const std::string&);

    friend bool operator==(const Edge& lhs, const Edge& rhs);
    friend bool operator!=(const Edge& lhs, const Edge& rhs);
};

#endif  // EDGE_HPP
