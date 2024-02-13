#include <algorithm>
#include <cctype>
#include <numeric>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "edge.hpp"
#include "menuitem.hpp"
#include "raylib.h"
#include "util.hpp"
#include "vertex.hpp"

int main() {
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    constexpr int fps = 60;

    constexpr float vertexRadius = 30;
    constexpr Color vertexGhostColor = GRAY;
    constexpr Color vertexColor = BLACK;

    constexpr int fontSizeSmall = 10;
    constexpr int fontSizeRegular = 14;
    constexpr int fontSizeLarge = 20;

    constexpr int menuItemWidth = 50;
    constexpr int menuItemHeight = 50;

    constexpr double visualisationDelay = 1.0;
    constexpr double afterVisualisationWaitTime = 5.0;
    constexpr Color currentVertexColor = RED;
    constexpr Color toVisitVertexColor = YELLOW;

    constexpr float edgeLineThickness = 2.0;

    Action currentAction = Action::Default;

    float mouseX{}, mouseY{};

    std::vector<Vertex> vertices;
    std::vector<Vertex> vertices_copy;

    std::vector<Edge> edges;
    Vector2 edgeStart, edgeEnd;

    std::unordered_map<std::string, std::vector<std::string>> adjacencyMap;

    bool mouseDown = false;
    bool actionSet = false;
    bool detailsOpen = false;

    std::variant<Vertex*, Edge*> currentSelection;
    Algorithm currentAlgorithm;

    Vector2 moveStart, moveEnd;

    int pressedKey{};

    bool searching = false;
    std::vector<std::string> searchTraverseOrder;
    std::map<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int>>>
        dijkstraSearchTraverseOrder;

    std::unordered_map<std::string, Vertex*> vertexMap;
    bool initmap = false;

    std::vector<MenuItem> menuItems{
        {{0, screenHeight / 2.0 - (4 * menuItemHeight), menuItemWidth, menuItemHeight},
         LIGHTGRAY,
         Action::Details},
        {{0, screenHeight / 2.0 - (3 * menuItemHeight), menuItemWidth, menuItemHeight},
         LIGHTGRAY,
         Action::Default},
        {{0, screenHeight / 2.0 - (2 * menuItemHeight), menuItemWidth, menuItemHeight},
         BLUE,
         Action::Vertex},
        {{0, screenHeight / 2.0 - (1 * menuItemHeight), menuItemWidth, menuItemHeight},
         GREEN,
         Action::Edge},
        {{0, screenHeight / 2.0 + (0 * menuItemHeight), menuItemWidth, menuItemHeight},
         RED,
         Action::WeightedEdge},
        {{0, screenHeight / 2.0 + (1 * menuItemHeight), menuItemWidth, menuItemHeight},
         YELLOW,
         Action::Search},
        {{0, screenHeight / 2.0 + (2 * menuItemHeight), menuItemWidth, menuItemHeight},
         PURPLE,
         Action::BFS,
         false},
        {{0 + menuItemWidth, screenHeight / 2.0 + (2 * menuItemHeight), menuItemWidth,
          menuItemHeight},
         BROWN,
         Action::DFS,
         false},
        {{0, screenHeight / 2.0 + (3 * menuItemHeight), menuItemWidth, menuItemHeight},
         MAROON,
         Action::Dijkstra,
         false}};

    InitWindow(screenWidth, screenHeight, "graphiz");
    SetTargetFPS(fps);

    while (!WindowShouldClose()) {
        if (searching) {
            if (currentAlgorithm == Algorithm::BFS || currentAlgorithm == Algorithm::DFS) {
                if (vertices_copy.size() == 0) {
                    vertices_copy = vertices;
                }

                BeginDrawing();
                ClearBackground(WHITE);

                for (auto& vertex : vertices_copy) {
                    if (vertex.usable) {
                        if (vertex.visited)
                            DrawCircle(vertex.pos.x, vertex.pos.y, vertex.radius, GREEN);
                        else
                            DrawCircle(vertex.pos.x, vertex.pos.y, vertex.radius, vertex.color);
                        if (!vertex.label.empty()) {
                            int textWidth = MeasureText(vertex.label.c_str(), fontSizeRegular);
                            DrawText(vertex.label.c_str(), vertex.pos.x - textWidth / 2.0,
                                     vertex.pos.y - 5, fontSizeRegular, WHITE);
                        }
                        if (!initmap) vertexMap[vertex.label] = &vertex;
                    }
                }
                if (!initmap) initmap = true;

                if (!edges.empty()) {
                    for (const auto& edge : edges) {
                        // unoptimized
                        auto fromVertex = std::find_if(
                            vertices.begin(), vertices.end(),
                            [&edge](const Vertex& vertex) { return edge.fromId == vertex.id; });
                        auto toVertex = std::find_if(
                            vertices.begin(), vertices.end(),
                            [&edge](const Vertex& vertex) { return edge.toId == vertex.id; });
                        if (fromVertex->usable && toVertex->usable) {
                            DrawLineEx({fromVertex->pos.x, fromVertex->pos.y},
                                       {toVertex->pos.x, toVertex->pos.y}, 3.0, BLACK);
                            if (!fromVertex->label.empty()) {
                                int textWidth =
                                    MeasureText(fromVertex->label.c_str(), fontSizeRegular);
                                DrawText(fromVertex->label.c_str(),
                                         fromVertex->pos.x - textWidth / 2.0, fromVertex->pos.y - 5,
                                         fontSizeRegular, WHITE);
                            }
                            if (!toVertex->label.empty()) {
                                int textWidth =
                                    MeasureText(toVertex->label.c_str(), fontSizeRegular);
                                DrawText(toVertex->label.c_str(), toVertex->pos.x - textWidth / 2.0,
                                         toVertex->pos.y - 5, fontSizeRegular, WHITE);
                            }
                        }
                    }
                }

                auto currentLabel = searchTraverseOrder.front();
                searchTraverseOrder.erase(searchTraverseOrder.begin());

                auto vertexWithCurrentLabel = vertexMap[currentLabel];

                DrawCircle(vertexWithCurrentLabel->pos.x, vertexWithCurrentLabel->pos.y,
                           vertexWithCurrentLabel->radius, currentVertexColor);

                int textWidth = MeasureText(vertexWithCurrentLabel->label.c_str(), fontSizeRegular);
                DrawText(vertexWithCurrentLabel->label.c_str(),
                         vertexWithCurrentLabel->pos.x - textWidth / 2.0,
                         vertexWithCurrentLabel->pos.y - 5, fontSizeRegular, WHITE);

                vertexWithCurrentLabel->visited = true;

                for (const auto& [key, values] : adjacencyMap) {
                    if (key == currentLabel)
                        for (auto& v : values) {
                            vertexMap[v]->color = toVisitVertexColor;
                        }
                }
            }

            switch (currentAlgorithm) {
                case Algorithm::BFS:
                    DrawText("Breadth-first search (BFS)", 5, 5, 20, BLACK);
                    DrawText("Time complexity: O(V+E)", 5, 25, 20, BLACK);
                    break;
                case Algorithm::DFS:
                    DrawText("Depth-first search (DFS)", 5, 5, 20, BLACK);
                    DrawText("Time complexity: O(V+E)", 5, 25, 20, BLACK);
                    break;
                case Algorithm::Dijkstra:
                    DrawText("Dijkstra", 5, 5, 20, BLACK);
                    DrawText("Time complexity: O(E+V log V)", 5, 25, 20, BLACK);
                    break;
            }
            DrawText(TextFormat("Delay: %.2fs", visualisationDelay), 5, 45, 20, BLACK);

            EndDrawing();
            if (!searchTraverseOrder.empty()) {
                WaitTime(visualisationDelay);
            } else {
                WaitTime(afterVisualisationWaitTime);
                searching = false;
                vertexMap.clear();
                vertices_copy.clear();
                initmap = false;
            }
        } else {
            BeginDrawing();

            ClearBackground(WHITE);

            /// Menu bar

            for (const auto& menuItem : menuItems) {
                if (menuItem.visible) {
                    switch (menuItem.action) {
                        case Action::Details:
                            DrawRectangleLines(menuItem.rect.x, menuItem.rect.y, menuItemWidth,
                                               menuItemHeight, menuItem.color);
                            DrawText(detailsOpen ? "ON" : "OFF", menuItem.rect.x + 15,
                                     menuItem.rect.y + menuItemHeight / 2.0 - 6, fontSizeRegular,
                                     BLACK);
                            break;
                        case Action::Default:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawLine(menuItem.rect.x + 35, menuItem.rect.y + 35,
                                     menuItem.rect.x + 15, menuItem.rect.y + 15, BLACK);
                            DrawLine(menuItem.rect.x + 15, menuItem.rect.y + 15,
                                     menuItem.rect.x + 30, menuItem.rect.y + 20, BLACK);
                            DrawLine(menuItem.rect.x + 15, menuItem.rect.y + 15,
                                     menuItem.rect.x + 20, menuItem.rect.y + 30, BLACK);
                            if (currentAction == Action::Default)
                                DrawRectangleLinesEx(
                                    {
                                        menuItem.rect.x,
                                        menuItem.rect.y,
                                        menuItemWidth,
                                        menuItemHeight,
                                    },
                                    edgeLineThickness + 1, BLACK);
                            break;
                        case Action::Vertex:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawCircle(menuItem.rect.x + menuItemWidth / 2.0,
                                       menuItem.rect.y + menuItemHeight / 2.0, 10, BLACK);
                            if (currentAction == Action::Vertex)
                                DrawRectangleLinesEx(
                                    {
                                        menuItem.rect.x,
                                        menuItem.rect.y,
                                        menuItemWidth,
                                        menuItemHeight,
                                    },
                                    edgeLineThickness + 1, BLACK);
                            break;
                        case Action::Edge:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawLine(menuItem.rect.x + 35, menuItem.rect.y + 35,
                                     menuItem.rect.x + 15, menuItem.rect.y + 15, BLACK);
                            if (currentAction == Action::Edge)
                                DrawRectangleLinesEx(
                                    {
                                        menuItem.rect.x,
                                        menuItem.rect.y,
                                        menuItemWidth,
                                        menuItemHeight,
                                    },
                                    edgeLineThickness + 1, BLACK);
                            break;
                        case Action::WeightedEdge:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawLine(menuItem.rect.x + 35, menuItem.rect.y + 35,
                                     menuItem.rect.x + 15, menuItem.rect.y + 15, BLACK);
                            DrawText("W", menuItem.rect.x + 27, menuItem.rect.y + 15, fontSizeSmall,
                                     BLACK);
                            if (currentAction == Action::WeightedEdge)
                                DrawRectangleLinesEx(
                                    {
                                        menuItem.rect.x,
                                        menuItem.rect.y,
                                        menuItemWidth,
                                        menuItemHeight,
                                    },
                                    edgeLineThickness + 1, BLACK);
                            break;
                        case Action::Search:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawText("Search", menuItem.rect.x + 6,
                                     menuItem.rect.y + menuItemHeight / 2.0 - 5, fontSizeSmall,
                                     BLACK);

                            if (currentAction == Action::Search) {
                                DrawRectangleLinesEx(
                                    {
                                        menuItem.rect.x,
                                        menuItem.rect.y,
                                        menuItemWidth,
                                        menuItemHeight,
                                    },
                                    edgeLineThickness + 1, BLACK);
                                (menuItems.end() - 1)->visible = true;
                                (menuItems.end() - 2)->visible = true;
                                (menuItems.end() - 3)->visible = true;
                            } else {
                                (menuItems.end() - 1)->visible = false;
                                (menuItems.end() - 2)->visible = false;
                                (menuItems.end() - 3)->visible = false;
                            }
                            break;
                        case Action::BFS:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawText("BFS", menuItem.rect.x + 15,
                                     menuItem.rect.y + menuItemHeight / 2.0 - 5, fontSizeRegular,
                                     BLACK);
                            break;
                        case Action::DFS:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawText("DFS", menuItem.rect.x + 15,
                                     menuItem.rect.y + menuItemHeight / 2.0 - 5, fontSizeRegular,
                                     BLACK);
                            break;
                        case Action::Dijkstra:
                            DrawRectangle(menuItem.rect.x, menuItem.rect.y, menuItem.rect.width,
                                          menuItem.rect.height, menuItem.color);
                            DrawText("DIJ", menuItem.rect.x + 15,
                                     menuItem.rect.y + menuItemHeight / 2.0 - 5, fontSizeRegular,
                                     BLACK);
                            break;
                    }
                }
            }

            /// end Menu bar

            //  Draw vertices
            if (!vertices.empty()) {
                if (!searching) {
                    auto v = tryGetVertex(currentSelection);
                    for (const auto& vertex : vertices) {
                        if (v.has_value()) {
                            auto currentVertex = v.value();
                            if (*currentVertex == vertex) {
                                DrawRectangleLines(vertex.pos.x - 35, vertex.pos.y - 35, 70, 70,
                                                   GREEN);

                                pressedKey = GetCharPressed();
                                std::regex initialLabel("V\\d");
                                while (pressedKey > 0) {
                                    if (isprint(pressedKey)) {
                                        if (std::regex_match(currentVertex->label, initialLabel))
                                            currentVertex->label.clear();
                                        currentVertex->label += pressedKey;
                                    }
                                    pressedKey = GetCharPressed();
                                }
                                if (IsKeyPressed(KEY_BACKSPACE)) {
                                    if (currentVertex->label.length() > 0)
                                        currentVertex->label.pop_back();
                                }
                                // Memory isn't freed from deleting
                                // vertices but rather the vertex just becomes
                                // unusable(hidden), which increases memory usage if a lot of
                                // vertices are created and "deleted" because edges hold id's of
                                // vertices and not references because of pointer misalignment
                                // while using std::vector, might be refactored to be more efficient
                                if (IsKeyPressed(KEY_X)) {
                                    if (currentVertex != nullptr) {
                                        int idToDelete = currentVertex->id;
                                        vertices[idToDelete].usable = false;
                                        edges.erase(
                                            std::remove_if(edges.begin(), edges.end(),
                                                           [idToDelete](const Edge& edge) {
                                                               return (edge.toId == idToDelete ||
                                                                       edge.fromId == idToDelete);
                                                           }),
                                            edges.end());
                                        resetCurrentSelection(currentSelection);
                                    }
                                }
                            }
                        }
                        if (vertex.usable) {
                            DrawCircle(vertex.pos.x, vertex.pos.y, vertex.radius, vertex.color);
                            if (!vertex.label.empty()) {
                                int textWidth = MeasureText(vertex.label.c_str(), fontSizeRegular);
                                DrawText(vertex.label.c_str(), vertex.pos.x - textWidth / 2.0,
                                         vertex.pos.y - 5, fontSizeRegular, WHITE);
                            }
                        }
                    }
                }
            }
            // Draw edges
            if (!edges.empty()) {
                if (!searching) {
                    int i = 0;
                    auto e = tryGetEdge(currentSelection);
                    for (const auto& edge : edges) {
                        if (e.has_value()) {
                            auto currentEdge = e.value();
                            if (*currentEdge == edge) {
                                pressedKey = GetCharPressed();
                                while (pressedKey > 0) {
                                    if (pressedKey >= 48 && pressedKey <= 57) {
                                        if (currentEdge->weight == "0") currentEdge->weight.clear();
                                        currentEdge->weight += pressedKey;
                                    } else if (pressedKey == 45 && currentEdge->weight.empty()) {
                                        currentEdge->weight += "-";
                                    }
                                    pressedKey = GetCharPressed();
                                }
                                if (IsKeyPressed(KEY_BACKSPACE)) {
                                    if (currentEdge->weight.length() > 0)
                                        currentEdge->weight.pop_back();
                                }
                                if (IsKeyPressed(KEY_X)) {
                                    if (currentEdge != nullptr) {
                                        int idToDelete = currentEdge->id;
                                        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                                                   [idToDelete](const Edge& edge) {
                                                                       return edge.id == idToDelete;
                                                                   }),
                                                    edges.end());
                                        resetCurrentSelection(currentSelection);
                                    }
                                }
                            }
                        }
                        // unoptimized
                        auto fromVertex = std::find_if(
                            vertices.begin(), vertices.end(),
                            [&edge](const Vertex& vertex) { return edge.fromId == vertex.id; });
                        auto toVertex = std::find_if(
                            vertices.begin(), vertices.end(),
                            [&edge](const Vertex& vertex) { return edge.toId == vertex.id; });
                        if (fromVertex->usable && toVertex->usable) {
                            DrawLineEx({fromVertex->pos.x, fromVertex->pos.y},
                                       {toVertex->pos.x, toVertex->pos.y}, edgeLineThickness,
                                       BLACK);
                            if (edge.weighted) {
                                int midX = (fromVertex->pos.x + toVertex->pos.x) / 2;
                                int midY = (fromVertex->pos.y + toVertex->pos.y) / 2;
                                const char* edgeWeight = edge.weight.c_str();
                                DrawRectangle(midX - 5, midY - 5,
                                              MeasureText(edgeWeight, fontSizeRegular) + 10, 20,
                                              BLACK);
                                DrawText(edgeWeight, midX, midY, fontSizeRegular, WHITE);
                            }
                            if (!fromVertex->label.empty()) {
                                int textWidth =
                                    MeasureText(fromVertex->label.c_str(), fontSizeRegular);
                                DrawText(fromVertex->label.c_str(),
                                         fromVertex->pos.x - textWidth / 2.0, fromVertex->pos.y - 5,
                                         fontSizeRegular, WHITE);
                            }
                            if (!toVertex->label.empty()) {
                                int textWidth =
                                    MeasureText(toVertex->label.c_str(), fontSizeRegular);
                                DrawText(toVertex->label.c_str(), toVertex->pos.x - textWidth / 2.0,
                                         toVertex->pos.y - 5, fontSizeRegular, WHITE);
                            }
                        }
                        // print edges
                        if (detailsOpen) {
                            DrawText(std::to_string(edge.fromId)
                                         .append(" -> ")
                                         .append(std::to_string(edge.toId))
                                         .append(edge.weighted ? " w: " : "")
                                         .append(edge.weighted ? edge.weight : "")
                                         .c_str(),
                                     screenWidth / 2.0, i, fontSizeRegular, RED);
                            i += 15;
                        }
                    }
                }
            }

            mouseX = GetMouseX();
            mouseY = GetMouseY();

            if (currentAction == Action::Vertex) {
                DrawCircle(mouseX, mouseY, vertexRadius, vertexGhostColor);
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                for (const auto& menuItem : menuItems) {
                    if (CheckCollisionPointRec({mouseX, mouseY}, menuItem.rect)) {
                        currentAction = menuItem.action;
                        auto currentVertex = tryGetVertex(currentSelection);
                        if (menuItem.isVisible()) {
                            if (!vertices.empty() && !edges.empty()) {
                            }
                            if (currentAction == Action::BFS) {
                                adjacencyMap = createAdjacencyMap(vertices, edges);
                                searchTraverseOrder =
                                    BFS(adjacencyMap, currentVertex.has_value()
                                                          ? currentVertex.value()->label
                                                          : "V0");

                                currentAlgorithm = Algorithm::BFS;
                                searching = true;
                                mouseDown = true;
                            } else if (currentAction == Action::DFS) {
                                adjacencyMap = createAdjacencyMap(vertices, edges);
                                searchTraverseOrder =
                                    DFS(adjacencyMap, currentVertex.has_value()
                                                          ? currentVertex.value()->label
                                                          : "V0");

                                currentAlgorithm = Algorithm::DFS;
                                searching = true;
                                mouseDown = true;
                            } else if (currentAction == Action::Dijkstra) {
                                if (std::any_of(edges.begin(), edges.end(),
                                                [](const Edge& edge) { return !edge.weighted; }))
                                    break;

                                auto z = createAdjacencyMapWeighted(vertices, edges);
                                dijkstraSearchTraverseOrder = Dijkstra(
                                    z, currentVertex.has_value() ? currentVertex.value()->label
                                                                 : "V0");

                                currentAlgorithm = Algorithm::Dijkstra;
                                searching = true;
                                mouseDown = true;
                            }
                        }
                        actionSet = true;
                        break;
                    }
                }
                if (!actionSet) {
                    switch (currentAction) {
                        case Action::Details:
                            if (!mouseDown) mouseDown = true;
                            break;
                        case Action::Default:
                            if (!mouseDown) {
                                bool alreadyFoundVertex = false;
                                for (const auto& vertex : vertices) {
                                    if (vertex.usable &&
                                        CheckCollisionPointCircle({mouseX, mouseY}, vertex.pos,
                                                                  vertex.radius)) {
                                        currentSelection = const_cast<Vertex*>(&vertex);
                                        moveStart.x = mouseX;
                                        moveStart.y = mouseY;
                                        mouseDown = true;
                                        alreadyFoundVertex = true;
                                        break;
                                    } else {
                                        resetCurrentSelection(currentSelection);
                                    }
                                }
                                if (!alreadyFoundVertex)
                                    for (const auto& edge : edges) {
                                        auto vertexFromWithId =
                                            std::find_if(vertices.begin(), vertices.end(),
                                                         [&edge](const Vertex& v) {
                                                             return v.id == edge.fromId;
                                                         });
                                        auto vertexToWithId = std::find_if(
                                            vertices.begin(), vertices.end(),
                                            [&edge](const Vertex& v) { return v.id == edge.toId; });
                                        if (CheckCollisionPointLine({mouseX, mouseY},
                                                                    vertexFromWithId->pos,
                                                                    vertexToWithId->pos, 15)) {
                                            currentSelection = const_cast<Edge*>(&edge);
                                            mouseDown = true;
                                            break;
                                        }
                                    }
                            }
                            break;
                        case Action::Vertex:
                            if (!mouseDown) {
                                vertices.push_back({{mouseX, mouseY}, vertexRadius, vertexColor});
                                mouseDown = true;
                            }
                            break;
                        case Action::WeightedEdge:
                            [[fallthrough]];
                        case Action::Edge:
                            if (!mouseDown) {
                                edgeStart.x = mouseX;
                                edgeStart.y = mouseY;
                                mouseDown = true;
                            } else
                                DrawLine(edgeStart.x, edgeStart.y, mouseX, mouseY, BLACK);
                            break;
                        case Action::Search:
                            [[fallthrough]];
                        case Action::BFS:
                            [[fallthrough]];
                        case Action::Dijkstra:
                            [[fallthrough]];
                        case Action::DFS:
                            if (!mouseDown) {
                                mouseDown = true;
                            }
                            break;
                    }
                }
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (actionSet) {
                    actionSet = false;
                    if (currentAction == Action::Details) {
                        detailsOpen = !detailsOpen;
                    }
                } else if (currentAction == Action::Default) {
                    auto currentVertexOrNull = tryGetVertex(currentSelection);
                    moveEnd.x = mouseX;
                    moveEnd.y = mouseY;
                    if (currentVertexOrNull.has_value() && (moveStart.x != moveEnd.x) &&
                        (moveStart.y != moveEnd.y)) {
                        currentVertexOrNull.value()->pos = moveEnd;
                    }
                    mouseDown = false;
                } else if (currentAction == Action::Vertex) {
                    mouseDown = false;
                } else if (currentAction == Action::Edge || currentAction == Action::WeightedEdge) {
                    edgeEnd.x = mouseX;
                    edgeEnd.y = mouseY;
                    int startVertexIndex = -1;
                    int endVertexIndex = -1;
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        if (vertices[i].usable) {
                            if (CheckCollisionPointCircle(edgeStart, vertices[i].pos,
                                                          vertices[i].radius))
                                startVertexIndex = i;
                            else if (CheckCollisionPointCircle(edgeEnd, vertices[i].pos,
                                                               vertices[i].radius))
                                endVertexIndex = i;
                        }
                    }
                    if (startVertexIndex != -1 && endVertexIndex != -1) {
                        bool exists = false;
                        for (const auto& edge : edges) {
                            if (edge.fromId == startVertexIndex && edge.toId == endVertexIndex) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            if (currentAction == Action::Edge)
                                edges.push_back({startVertexIndex, endVertexIndex});
                            else
                                edges.push_back({startVertexIndex, endVertexIndex, "0"});
                        }
                    }
                    mouseDown = false;
                } else if (currentAction == Action::Search || currentAction == Action::BFS ||
                           currentAction == Action::DFS || currentAction == Action::Dijkstra) {
                    mouseDown = false;
                }
            }

            if (detailsOpen) {
                DrawFPS(5, 5);
                // i love ternaries
                DrawText(currentAction == Action::Vertex         ? "Mode: Vertex"
                         : currentAction == Action::Edge         ? "Mode: Edge"
                         : currentAction == Action::WeightedEdge ? "Mode: Weighted edge"
                         : currentAction == Action::Search       ? "Mode: Search"
                                                                 : "Mode: Default",
                         5, 25, fontSizeLarge, BLACK);

                int numOfVerticesOnScreen = std::accumulate(
                    vertices.begin(), vertices.end(), 0,
                    [](int current, const Vertex& v) { return current + (v.usable ? 1 : 0); });

                DrawText(TextFormat("Vertices: %d", numOfVerticesOnScreen), 5, 45, fontSizeLarge,
                         BLACK);
                DrawText(TextFormat("Edges: %d", edges.size()), 5, 65, fontSizeLarge, BLACK);

                auto v = tryGetVertex(currentSelection);
                auto e = tryGetEdge(currentSelection);
                if (v.has_value()) {
                    DrawText(TextFormat("Selected vertex id: %d", v.value()->id), 5, 85,
                             fontSizeLarge, RED);
                } else if (e.has_value()) {
                    DrawText(TextFormat("Selected edge id: %d", e.value()->id), 5, 85,
                             fontSizeLarge, RED);
                }
            }

            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}
