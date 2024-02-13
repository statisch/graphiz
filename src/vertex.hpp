#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <raylib.h>

#include <string>

struct Vertex {
    static int instanceCounter;
    int id;
    Vector2 pos;
    float radius;
    Color color;
    std::string label;
    bool usable;
    int visualisation_currently_active;
    bool visited;

    Vertex();
    Vertex(const Vector2& pos, float radius, const Color& color);
    Vertex(const Vector2& pos, float radius, const Color& color, const std::string& label);

    friend bool operator==(const Vertex& lhs, const Vertex& rhs);
    friend bool operator!=(const Vertex& lhs, const Vertex& rhs);
};

#endif  // VERTEX_HPP
