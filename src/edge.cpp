#include "edge.hpp"

#include <string>

int Edge::instanceCounter = 0;

Edge::Edge(int fromId, int toId) {
    this->id = instanceCounter++;
    this->fromId = fromId;
    this->toId = toId;
}

Edge::Edge(int fromId, int toId, const std::string& weight) {
    this->id = instanceCounter++;
    this->fromId = fromId;
    this->toId = toId;
    this->weight = weight;
    this->weighted = true;
}

bool operator==(const Edge& lhs, const Edge& rhs) { return lhs.id == rhs.id; }
bool operator!=(const Edge& lhs, const Edge& rhs) { return lhs.id != rhs.id; }
