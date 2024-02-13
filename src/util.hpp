#ifndef UTIL_HPP
#define UTIL_HPP

#include <raylib.h>

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "edge.hpp"
#include "vertex.hpp"

enum class Algorithm { BFS, DFS, Dijkstra };

std::unordered_map<std::string, std::vector<std::string>> createAdjacencyMap(
    const std::vector<Vertex>&, const std::vector<Edge>&);

std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
createAdjacencyMapWeighted(const std::vector<Vertex>&, const std::vector<Edge>&);

std::vector<std::string> BFS(const std::unordered_map<std::string, std::vector<std::string>>&,
                             const std::string&);

std::vector<std::string> DFS(const std::unordered_map<std::string, std::vector<std::string>>&,
                             const std::string&);

// bad, "three star c programmer" equivalent for c++
std::map<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int>>> Dijkstra(
    const std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>&,
    const std::string&);

std::optional<Vertex*> tryGetVertex(const std::variant<Vertex*, Edge*>&);
std::optional<Edge*> tryGetEdge(const std::variant<Vertex*, Edge*>&);

void resetCurrentSelection(std::variant<Vertex*, Edge*>&);

#endif  // UTIL_HPP
