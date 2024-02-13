#include "util.hpp"

#include <limits>
#include <map>
#include <optional>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

std::unordered_map<std::string, std::vector<std::string>> createAdjacencyMap(
    const std::vector<Vertex>& vertices, const std::vector<Edge>& edges) {
    std::unordered_map<std::string, std::vector<std::string>> adjacencyMap;

    std::unordered_map<int, const Vertex*> vertexMap;
    for (const auto& vertex : vertices) {
        if (vertex.usable) vertexMap[vertex.id] = &vertex;
    }

    adjacencyMap.reserve(edges.size());

    for (const Edge& edge : edges) {
        auto fromIter = vertexMap.find(edge.fromId);
        auto toIter = vertexMap.find(edge.toId);

        if (fromIter != vertexMap.end() && toIter != vertexMap.end()) {
            adjacencyMap[fromIter->second->label].push_back(toIter->second->label);
        }
    }

    return adjacencyMap;
}

std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
createAdjacencyMapWeighted(const std::vector<Vertex>& vertices, const std::vector<Edge>& edges) {
    std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> adjacencyMap;

    std::unordered_map<int, const Vertex*> vertexMap;
    for (const auto& vertex : vertices) {
        if (vertex.usable) {
            vertexMap[vertex.id] = &vertex;
            if (adjacencyMap[vertex.label].empty()) {
                adjacencyMap[vertex.label] = {};
            }
        }
    }

    for (const Edge& edge : edges) {
        auto fromIter = vertexMap.find(edge.fromId);
        auto toIter = vertexMap.find(edge.toId);

        if (fromIter != vertexMap.end() && toIter != vertexMap.end()) {
            adjacencyMap[fromIter->second->label].push_back(
                {std::stoi(edge.weight), toIter->second->label});
        }
    }

    return adjacencyMap;
}

std::vector<std::string> BFS(
    const std::unordered_map<std::string, std::vector<std::string>>& adjacencyMap,
    const std::string& startVertex) {
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;
    std::string current;
    std::vector<std::string> returnVec;

    q.push(startVertex);
    visited.insert(startVertex);

    while (!q.empty()) {
        current = q.front();
        q.pop();

        returnVec.push_back(current);

        if (adjacencyMap.find(current) != adjacencyMap.end())
            for (const auto& adjacent : adjacencyMap.at(current)) {
                if (visited.emplace(adjacent).second) {
                    q.push(adjacent);
                }
            }
    }

    return returnVec;
}

std::vector<std::string> DFS(
    const std::unordered_map<std::string, std::vector<std::string>>& adjacencyMap,
    const std::string& startVertex) {
    std::unordered_set<std::string> visited;
    std::stack<std::string> st;
    std::string current;
    std::vector<std::string> returnVec;

    st.push(startVertex);
    visited.insert(startVertex);

    while (!st.empty()) {
        current = st.top();
        st.pop();

        returnVec.push_back(current);

        if (adjacencyMap.find(current) != adjacencyMap.end())
            for (const auto& adjacent : adjacencyMap.at(current))
                if (visited.emplace(adjacent).second) {
                    st.push(adjacent);
                }
    }

    return returnVec;
}

struct Compare {
    constexpr bool operator()(const std::pair<int, std::string>& lhs,
                              const std::pair<int, std::string>& rhs) const {
        return lhs.first > rhs.first;
    };
};

std::map<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int>>> Dijkstra(
    const std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>& adjacencyMap,
    const std::string& startVertex) {
    std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>,
                        Compare>
        pq;

    std::map<std::string, int> dist;

    std::map<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int>>>
        returnMap;

    std::string adjacentVertex;
    std::string currentVertex;
    int adjacentVertexWeight;

    for (const auto& entry : adjacencyMap) {
        dist[entry.first] = std::numeric_limits<int>::max();
    }

    dist[startVertex] = 0;
    pq.push({0, startVertex});

    while (!pq.empty()) {
        int dis = pq.top().first;
        currentVertex = pq.top().second;
        pq.pop();

        if (adjacencyMap.find(currentVertex) != adjacencyMap.end())
            for (auto it : adjacencyMap.at(currentVertex)) {
                adjacentVertexWeight = it.first;
                adjacentVertex = it.second;

                if (dis + adjacentVertexWeight < dist[adjacentVertex]) {
                    dist[adjacentVertex] = dis + adjacentVertexWeight;
                    pq.push({dist[adjacentVertex], adjacentVertex});
                }

                returnMap.insert({currentVertex, {{{adjacentVertex, adjacentVertexWeight}}, dist}});
            }
    }
    returnMap.insert({currentVertex, {{{adjacentVertex, adjacentVertexWeight}}, dist}});

    return returnMap;
}

std::optional<Vertex*> tryGetVertex(const std::variant<Vertex*, Edge*>& selection) {
    if (std::holds_alternative<Vertex*>(selection)) {
        if (std::get<Vertex*>(selection) != nullptr)
            return (*std::make_optional(std::get<Vertex*>(selection)));
        else
            return std::nullopt;
    } else {
        return std::nullopt;
    }
}

std::optional<Edge*> tryGetEdge(const std::variant<Vertex*, Edge*>& selection) {
    if (std::holds_alternative<Edge*>(selection)) {
        if (std::get<Edge*>(selection) != nullptr)
            return (*std::make_optional(std::get<Edge*>(selection)));
        else
            return std::nullopt;
    } else {
        return std::nullopt;
    }
}

// Probably bad but only one that works to reset currentSelection
void resetCurrentSelection(std::variant<Vertex*, Edge*>& currentSelection) {
    currentSelection = std::variant<Vertex*, Edge*>{};
}
