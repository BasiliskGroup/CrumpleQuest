#include "levels/navmesh.h"
#include <earcut.hpp>


void Navmesh::earcut(const std::vector<std::vector<vec2>>& polygon, std::vector<uint>& indices) {
    std::vector<std::vector<std::array<double, 2>>> polyDouble;
    polyDouble.resize(polygon.size());

    for (uint i = 0; i < polygon.size(); i++) {
        for (uint j = 0; j < polygon.at(i).size(); j++) {
            vec2 v = polygon[i][j];
            polyDouble[i].push_back({v.x, v.y});
        }   
    }

    indices = mapbox::earcut(polyDouble);
}

void Navmesh::convertToMesh(
    const std::vector<std::vector<vec2>>& polygon,
    std::vector<uint>& indices,
    std::vector<float>& data)
{
    data.clear();

    data.reserve(indices.size() * 5);

    std::vector<vec2> polyFlat;

    // flatten array
    for (uint i = 0; i < polygon.size(); i++) {
        for (uint j = 0; j < polygon.at(i).size(); j++) {
            vec2 v = polygon[i][j];
            polyFlat.push_back({v.x, v.y});
        }   
    }

    for (size_t i = 0; i < indices.size(); ++i) {
        vec2 v = polyFlat[indices[i]];
        size_t m = i % 3;

        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(0.0f);
        data.push_back(m == 2 ? 1.0f : 0.0f);
        data.push_back(m == 1 ? 1.0f : 0.0f);
    }
}
