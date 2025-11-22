#include "edger.h"
#include <stdexcept>

Edger::Edger(const std::vector<vec2> region) : region(region) {}

vec2 Edger::getNearestEdgeIntersection(const vec2& pos, const vec2& dir) {
    float closestT = std::numeric_limits<float>::max();
    vec2 closestPoint = pos;

    size_t n = region.size();
    if (n < 2)
        return pos; // no edges

    for (size_t i = 0; i < n; ++i) {
        const vec2& a = region[i];
        const vec2& b = region[(i + 1) % n];
        vec2 hit;

        if (intersectLineSegmentInfiniteLine(a, b, pos, dir, hit)) {
            float t = glm::dot(hit - pos, dir);
            if (t > 0 && t < closestT) {
                closestT = t;
                closestPoint = hit;
            }
        }
    }

    return closestPoint;
}

vec2 Edger::getNearestEdgePoint(const vec2& pos) {
    size_t n = region.size();
    if (n < 2) return pos;

    float bestDistSq = std::numeric_limits<float>::max();
    vec2 bestPoint = pos;

    for (size_t i = 0; i < n; ++i) {
        const vec2& a = region[i];
        const vec2& b = region[(i + 1) % n];
        vec2 candidate = nearestPointOnEdgeToPoint(a, b, pos);
        float distSq = glm::dot(candidate - pos, candidate - pos);

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestPoint = candidate;
        }
    }

    return bestPoint;
}

std::pair<int, int> Edger::getVertexRangeBelowThreshold(const vec2& dir, float thresh, const vec2& start) {
    size_t n = region.size();
    if (n == 0) {
        throw std::runtime_error("Cannot get vertex range on empty vertex list");
    }

    // Step 1: Find the vertex closest to start
    float minDistSq = std::numeric_limits<float>::max();
    size_t closestIndex = 0;
    
    for (size_t i = 0; i < n; ++i) {
        float distSq = glm::dot(region[i] - start, region[i] - start);
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closestIndex = i;
        }
    }

    // Verify that the closest vertex satisfies the threshold condition
    float closestDot = glm::dot(region[closestIndex], dir);
    if (closestDot >= thresh) {
        throw std::runtime_error("Closest vertex dot product is not less than threshold");
    }

    // Step 2: Walk CCW (backwards in index) from closest until we find a vertex >= thresh
    // This finds the first vertex OUTSIDE the region on the CCW side
    int currentIndex = static_cast<int>(closestIndex);
    int leftFirstOutside = -1;
    size_t stepsLeft = 0;
    
    while (stepsLeft < n) {
        int prevIndex = (currentIndex == 0) ? static_cast<int>(n - 1) : (currentIndex - 1);
        float prevDot = glm::dot(region[prevIndex], dir);
        
        if (prevDot >= thresh) {
            // Found first vertex outside going CCW
            leftFirstOutside = prevIndex;
            break;
        }
        
        currentIndex = prevIndex;
        stepsLeft++;
    }
    
    // If we walked all vertices, all are below threshold
    if (stepsLeft == n) {
        return {0, static_cast<int>(n - 1)};
    }

    // Step 3: Walk CW (forwards in index) from closest until we find a vertex >= thresh
    // This finds the first vertex OUTSIDE the region on the CW side
    currentIndex = static_cast<int>(closestIndex);
    int rightFirstOutside = -1;
    size_t stepsRight = 0;
    
    while (stepsRight < n) {
        int nextIndex = (currentIndex + 1) % static_cast<int>(n);
        float nextDot = glm::dot(region[nextIndex], dir);
        
        if (nextDot >= thresh) {
            // Found first vertex outside going CW
            rightFirstOutside = nextIndex;
            break;
        }
        
        currentIndex = nextIndex;
        stepsRight++;
    }
    
    int first = (leftFirstOutside + 1) % static_cast<int>(n);
    int second = (rightFirstOutside - 1 + static_cast<int>(n)) % static_cast<int>(n);
    
    return {first, second};
}

void Edger::addRangeInside(std::vector<vec2>& vecs, const std::pair<int, int> range) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    int a = glm::clamp(range.first, 0, n - 1);
    int b = glm::clamp(range.second, 0, n - 1);

    // Collect indices in order, handling wrap-around if b < a
    for (int i = a;; i = (i + 1) % n) {
        vecs.push_back(region[i]);
        if (i == b) break;
    }
}

bool Edger::getEdgeIntersection(int edgeStartIndex, const vec2& pos, const vec2& dir, vec2& out) {
    size_t n = region.size();
    if (n < 2) {
        return false; // Need at least 2 vertices to form an edge
    }

    // Wrap the indices to handle out-of-bounds and negative values
    // For negative indices, we need to add multiples of n until positive
    int wrappedStart = edgeStartIndex % static_cast<int>(n);
    if (wrappedStart < 0) {
        wrappedStart += n;
    }
    
    size_t startIdx = static_cast<size_t>(wrappedStart);
    size_t endIdx = (startIdx + 1) % n;

    const vec2& edgeStart = region[startIdx];
    const vec2& edgeEnd = region[endIdx];

    if (intersectLineSegmentInfiniteLine(edgeStart, edgeEnd, pos, dir, out)) {
        return true;
    }

    return false; // No intersection found
}

void Edger::reflectVerticesOverLine(std::vector<vec2>& reflected, int a, int b, const vec2& pos, const vec2& dir) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    a = glm::clamp(a, 0, n - 1);
    b = glm::clamp(b, 0, n - 1);

    // Collect indices in order, handling wrap-around if b < a
    std::vector<int> indices;
    for (int i = a;; i = (i + 1) % n) {
        indices.push_back(i);
        if (i == b) break;
    }

    // Reserve extra space without invalidating existing contents
    reflected.reserve(reflected.size() + indices.size());

    // Append reflected vertices in reverse order (so result is reversed),
    // without modifying any existing elements in 'reflected'.
    for (int idx = static_cast<int>(indices.size()) - 1; idx >= 0; --idx) {
        int vindex = indices[idx];
        reflected.push_back(reflectPointOverLine(pos, dir, region[vindex]));
    }
}

void Edger::addRangeOutside(std::vector<vec2>& unreflected, int a, int b) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    a = glm::clamp(a, 0, n - 1);
    b = glm::clamp(b, 0, n - 1);

    // Collect everything *outside* the [a, b] segment
    for (int i = (b + 1) % n;; i = (i + 1) % n) {
        unreflected.push_back(region[i]);
        if (i == (a - 1 + n) % n)  // stop right before 'a'
            break;
    }
}

// testing functions
bool Edger::isPointOutside(const vec2& p, float eps) const {
    size_t n = region.size();
    if (n < 3) return true; // degenerate polygon → everything outside

    bool inside = false;

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const vec2& a = region[i];
        const vec2& b = region[j];

        // --- EPS boundary check: if p is near edge → treat as inside ---
        vec2 ab = b - a;
        float abLen2 = dot(ab, ab);

        float t = glm::clamp(dot(p - a, ab) / (abLen2 + 1e-20f), 0.0f, 1.0f);
        vec2 proj = a + t * ab;

        // Distance to the edge or the vertices
        if (length(p - proj) <= eps) return false; // NOT outside
        if (length(p - a) <= eps) return false;    // NOT outside
        if (length(p - b) <= eps) return false;    // NOT outside

        // --- Standard even–odd raycast test ---
        bool intersect =
            ((a.y > p.y) != (b.y > p.y)) &&
            (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-12f) + a.x);

        if (intersect)
            inside = !inside;
    }

    // If inside → not outside
    return !inside;
}

void Edger::removeAll(const std::vector<vec2> removes, float epsilon) {
    if (region.empty() || removes.empty()) return;

    float epsSq = epsilon * epsilon;

    std::vector<vec2> filtered;
    filtered.reserve(region.size());

    for (const vec2& v : region) {
        bool tooClose = false;

        for (const vec2& r : removes) {
            vec2 d = v - r;
            if (glm::dot(d, d) <= epsSq) {
                tooClose = true;
                break;
            }
        }

        if (!tooClose) {
            filtered.push_back(v);
        }
    }

    region.swap(filtered);
}

void Edger::pruneDups() {
    if (region.size() <= 1) return;
    
    const float eps = 1e-6f;
    
    for (int i = 0; i < region.size(); i++) {
        for (int j = i + 1; j < region.size(); j++) {
            if (glm::all(glm::epsilonEqual(region[i], region[j], eps))) {
                region.erase(region.begin() + j);
                j--; 
            }
        }
    }
}

void Edger::flipHorizontal() {
    flipVecsHorizontal(region);
}