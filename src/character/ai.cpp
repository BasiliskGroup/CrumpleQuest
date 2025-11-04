#include "character/ai.h"

AI::AI(Navmesh* navmesh) : navmesh(navmesh) {}

vec2 AI::targetPosition(const vec2& start) {
    if (path.size() < 2) return start;
    return path.at(1);
}

vec2 AI::targetDirection(const vec2& start) {
    if (path.size() < 2) return start;
    return path.at(1);
}

void AI::updatePath(const vec2& start, const vec2& dest) {
    navmesh->getPath(path, start, dest);
}