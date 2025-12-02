#include "levels/uvregion.h"

UVRegion::UVRegion(const std::vector<vec2>& positions, const std::array<Vert, 2>& basis, const vec2& originUV, bool isObstacle) 
    : positions(positions), basis(basis), originUV(originUV), isObstacle(isObstacle) {
}

UVRegion::UVRegion(Mesh* mesh, const vec3& position, const vec2& scale, bool isObstacle)
    : positions(), basis(), originUV(0.0f), isObstacle(isObstacle)
{
    // Transform components
    vec2 pos = vec2(position.x, position.y);
    float r  = position.z;

    float s = std::sin(r);
    float c = std::cos(r);

    auto rot = [&](const vec2& v) {
        return vec2(c * v.x - s * v.y,
                    s * v.x + c * v.y);
    };

    auto& verts = mesh->getVertices();      // [x,y,z,u,v,...]
    const int vstride = 5;
    const int numVerts = verts.size() / vstride;

    positions.reserve(numVerts);

    std::vector<vec2> localPos;
    std::vector<vec2> uv;
    localPos.reserve(numVerts);
    uv.reserve(numVerts);

    // Load vertex data
    for (int i = 0; i < numVerts; i++) {
        float lx = verts[i*vstride + 0];
        float ly = verts[i*vstride + 1];
        float ux = verts[i*vstride + 3];
        float uy = verts[i*vstride + 4];

        vec2 pLocal = vec2(lx, ly) * scale;
        vec2 pWorld = rot(pLocal) + pos;

        localPos.push_back(pLocal);
        uv.push_back(vec2(ux, uy));
        positions.push_back(pWorld);
    }

    if (numVerts == 0)
        return;

    // Origin UV
    originUV = uv[0];

    // Find two non-collinear edges for basis
    int i1 = -1, i2 = -1;

    for (int i = 1; i < numVerts; i++) {
        if ((localPos[i] - localPos[0]).length() > 1e-6f) {
            i1 = i;
            break;
        }
    }
    for (int i = 1; i < numVerts; i++) {
        if (i != i1 &&
            std::abs(cross(localPos[i] - localPos[0],
                           localPos[i1] - localPos[0])) > 1e-6f)
        {
            i2 = i;
            break;
        }
    }

    // Degenerate fallback
    if (i1 < 0 || i2 < 0) {
        basis[0].pos = vec2(1, 0);
        basis[0].uv  = vec2(1, 0);
        basis[1].pos = vec2(0, 1);
        basis[1].uv  = vec2(0, 1);
        return;
    }

    // Construct basis
    vec2 p0  = positions[0];
    vec2 p1w = positions[i1];
    vec2 p2w = positions[i2];

    vec2 uv0  = uv[0];
    vec2 uv1  = uv[i1];
    vec2 uv2  = uv[i2];

    basis[0].pos = p1w - p0;
    basis[0].uv  = uv1 - uv0;

    basis[1].pos = p2w - p0;
    basis[1].uv  = uv2 - uv0;
}

bool UVRegion::contains(const vec2& p, float eps) const {
    size_t n = positions.size();
    if (n < 3) return false;

    // Check if point is on boundary (within epsilon)
    for (size_t i = 0; i < n; ++i) {
        const vec2& a = positions[i];
        const vec2& b = positions[(i + 1) % n];
        
        vec2 ab = b - a;
        float abLen2 = glm::dot(ab, ab);
        float t = glm::clamp(glm::dot(p - a, ab) / (abLen2 + 1e-20f), 0.0f, 1.0f);
        vec2 proj = a + t * ab;
        
        if (glm::length(p - proj) <= eps) return true;
        if (glm::length(p - a) <= eps) return true;
        if (glm::length(p - b) <= eps) return true;
    }

    // Standard even-odd raycast test
    bool inside = false;
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const vec2& a = positions[i];
        const vec2& b = positions[j];
        
        bool intersect = ((a.y > p.y) != (b.y > p.y)) &&
                        (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-12f) + a.x);
        if (intersect) inside = !inside;
    }
    
    return inside;
}

float UVRegion::distance(const vec2& pos) const {
    if (contains(pos, 0.0f)) return 0.0f;
    
    size_t n = positions.size();
    float minDist = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < n; ++i) {
        const vec2& a = positions[i];
        const vec2& b = positions[(i + 1) % n];
        
        vec2 ab = b - a;
        float t = glm::clamp(glm::dot(pos - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
        vec2 closest = a + t * ab;
        
        float dist = glm::length(pos - closest);
        minDist = std::min(minDist, dist);
    }
    
    return minDist;
}

vec2 UVRegion::sampleUV(const vec2& pos) const {
    // Affine transformation from position space to UV space:
    // pos_relative = pos - origin (where origin = positions[0])
    // pos_relative = a * basis[0].pos + b * basis[1].pos
    // uv = originUV + a * basis[0].uv + b * basis[1].uv
    
    if (positions.empty()) return originUV;
    
    const vec2& origin = positions[0];
    const vec2& d0 = basis[0].pos;  // direction vector 1
    const vec2& d1 = basis[1].pos;  // direction vector 2
    
    // Express (pos - origin) as linear combination of basis directions
    vec2 rel = pos - origin;
    
    // Solve 2x2 system: [d0.x d1.x] [a]   [rel.x]
    //                   [d0.y d1.y] [b] = [rel.y]
    float det = d0.x * d1.y - d1.x * d0.y;
    if (std::abs(det) < 1e-12f) {
        return originUV;  // Degenerate basis, return origin UV
    }
    
    float a = (rel.x * d1.y - rel.y * d1.x) / det;
    float b = (d0.x * rel.y - d0.y * rel.x) / det;
    
    return originUV + a * basis[0].uv + b * basis[1].uv;
}

void UVRegion::flipUVx() {
    // Negate the x component of origin and both basis UVs
    originUV.x = -originUV.x;
    basis[0].uv.x = -basis[0].uv.x;
    basis[1].uv.x = -basis[1].uv.x;
}

void UVRegion::flipHorizontal() {
    for (vec2& p : positions) {
        p.x *= -1.0f;
    }
    // Also flip the basis positions
    basis[0].pos.x *= -1.0f;
    basis[1].pos.x *= -1.0f;
}