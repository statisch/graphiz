#include "vertex.hpp"

#include <string>

int Vertex::instanceCounter = 0;

Vertex::Vertex() {}

Vertex::Vertex(const Vector2& pos, float radius, const Color& color) {
    id = instanceCounter++;
    this->pos = pos;
    this->radius = radius;
    this->color = color;
    this->label = "V" + std::to_string(id);
    this->usable = true;
    this->visualisation_currently_active = false;
    this->visited = false;
}

Vertex::Vertex(const Vector2& pos, float radius, const Color& color, const std::string& label) {
    id = instanceCounter++;
    this->pos = pos;
    this->radius = radius;
    this->color = color;
    this->label = label;
    this->usable = true;
    this->visualisation_currently_active = false;
    this->visited = false;
}

bool operator==(const Vertex& lhs, const Vertex& rhs) { return lhs.id == rhs.id; }
bool operator!=(const Vertex& lhs, const Vertex& rhs) { return lhs.id != rhs.id; }
