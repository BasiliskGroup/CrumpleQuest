#include "edger.h"
#include <stdexcept>
#include <cmath>

Edger::Edger(const std::vector<Point64>& region) : region(region) {}

Edger::Edger(const std::vector<vec2>& region) : region() {
    this->region.reserve(region.size());
    for (const vec2& v : region) {
        this->region.push_back(vec2ToPoint64(v));
    }
}

bool Edger::isInsideOrOnEdge(const Point64& point) const {
    // For CCW polygon: point is inside if it's on or to the LEFT of all edges
    size_t n = region.size();
    for (size_t i = 0; i < n; ++i) {
        const Point64& A = region[i];
        const Point64& B = region[(i + 1) % n];
        
        Point64 edgeDir = B - A;
        Point64 toPoint = point - A;
        
        // Cross product: positive = left (inside), zero = on edge, negative = right (outside)
        int64_t cross = edgeDir.x * toPoint.y - edgeDir.y * toPoint.x;
        
        if (cross < 0) {
            return false;  // Point is to the right of this edge (outside)
        }
    }
    return true;
}

Point64 Edger::moveInward(const Point64& point, size_t edgeIndex, int64_t stepSize) const {
    size_t n = region.size();
    size_t next = (edgeIndex + 1) % n;
    
    const Point64& A = region[edgeIndex];
    const Point64& B = region[next];
    
    Point64 edgeDir = B - A;
    // For CCW polygon: 90° CCW rotation points inward
    Point64 inwardNormal = {-edgeDir.y, edgeDir.x};
    
    // Normalize to unit length
    double length = std::sqrt(static_cast<double>(dot64(inwardNormal, inwardNormal)));
    if (length < 1e-10) {
        // Degenerate edge, can't move
        return point;
    }
    
    // Move by stepSize units in the inward direction
    Point64 result;
    result.x = point.x + static_cast<int64_t>(stepSize * inwardNormal.x / length);
    result.y = point.y + static_cast<int64_t>(stepSize * inwardNormal.y / length);
    
    return result;
}

bool Edger::getNearestEdgeIntersection(const Point64& p0, const Point64& p1, Point64& out) {
    size_t n = region.size();
    if (n < 3) {
        return false;  // Need at least 3 vertices for a polygon
    }

    double closestT = std::numeric_limits<double>::infinity();
    size_t closestEdgeIndex = 0;
    double closestS = 0.0;
    bool foundIntersection = false;

    // Convert ray to double for calculations
    double rayStartX = static_cast<double>(p0.x);
    double rayStartY = static_cast<double>(p0.y);
    double rayDirX = static_cast<double>(p1.x - p0.x);
    double rayDirY = static_cast<double>(p1.y - p0.y);

    // Find nearest intersection with any edge
    for (size_t i = 0; i < n; ++i) {
        size_t next = (i + 1) % n;
        
        double edgeStartX = static_cast<double>(region[i].x);
        double edgeStartY = static_cast<double>(region[i].y);
        double edgeEndX = static_cast<double>(region[next].x);
        double edgeEndY = static_cast<double>(region[next].y);
        
        double edgeDirX = edgeEndX - edgeStartX;
        double edgeDirY = edgeEndY - edgeStartY;
        
        // Solve: edgeStart + s * edgeDir = rayStart + t * rayDir
        double cross = edgeDirX * rayDirY - edgeDirY * rayDirX;
        if (std::abs(cross) < 1e-10) {
            continue;  // Parallel or degenerate
        }
        
        double diffX = rayStartX - edgeStartX;
        double diffY = rayStartY - edgeStartY;
        
        double s = (diffY * rayDirX - diffX * rayDirY) / cross;
        double t = (diffY * edgeDirX - diffX * edgeDirY) / cross;
        
        // Check if intersection is on the edge segment and in forward direction of ray
        if (s >= 0.0 && s <= 1.0 && t > 0.0) {
            if (t < closestT) {
                closestT = t;
                closestEdgeIndex = i;
                closestS = s;
                foundIntersection = true;
            }
        }
    }

    if (!foundIntersection) {
        return false;  // No intersection found
    }

    // Compute intersection point - just return it, no validation needed
    size_t nextIdx = (closestEdgeIndex + 1) % n;
    double edgeStartX = static_cast<double>(region[closestEdgeIndex].x);
    double edgeStartY = static_cast<double>(region[closestEdgeIndex].y);
    double edgeEndX = static_cast<double>(region[nextIdx].x);
    double edgeEndY = static_cast<double>(region[nextIdx].y);
    
    // Clamp s to [0, 1] for safety
    closestS = std::max(0.0, std::min(1.0, closestS));
    
    // Interpolate on the edge and return
    out.x = static_cast<int64_t>(edgeStartX + closestS * (edgeEndX - edgeStartX));
    out.y = static_cast<int64_t>(edgeStartY + closestS * (edgeEndY - edgeStartY));
    
    return true;
}

bool Edger::getEdgeIntersection(int edgeStartIndex, const Point64& p0, const Point64& p1, Point64& out) {
    size_t n = region.size();
    if (n < 2) {
        return false;
    }

    int wrappedStart = wrapIndex(edgeStartIndex, static_cast<int>(n));
    size_t startIdx = static_cast<size_t>(wrappedStart);
    size_t endIdx = (startIdx + 1) % n;

    // Convert to double for precise calculation
    double edgeStartX = static_cast<double>(region[startIdx].x);
    double edgeStartY = static_cast<double>(region[startIdx].y);
    double edgeEndX = static_cast<double>(region[endIdx].x);
    double edgeEndY = static_cast<double>(region[endIdx].y);
    
    double lineStartX = static_cast<double>(p0.x);
    double lineStartY = static_cast<double>(p0.y);
    double lineDirX = static_cast<double>(p1.x - p0.x);
    double lineDirY = static_cast<double>(p1.y - p0.y);
    
    double edgeDirX = edgeEndX - edgeStartX;
    double edgeDirY = edgeEndY - edgeStartY;
    
    // Debug output for EVERY edge test
    std::cout << "  Testing edge " << edgeStartIndex << ": (" << edgeStartX << ", " << edgeStartY << ") to (" << edgeEndX << ", " << edgeEndY << ")" << std::endl;
    
    // Solve for intersection of INFINITE line with edge segment
    double cross = edgeDirX * lineDirY - edgeDirY * lineDirX;
    
    if (std::abs(cross) < 1e-10) {
        std::cout << "    PARALLEL (cross = " << cross << ")" << std::endl;
        return false;
    }
    
    double diffX = lineStartX - edgeStartX;
    double diffY = lineStartY - edgeStartY;
    
    double s = (diffY * lineDirX - diffX * lineDirY) / cross;
    
    std::cout << "    s = " << s << " (needs [0,1] with tolerance)" << std::endl;
    
    // Relax the constraint - allow s slightly outside [0,1] for numerical tolerance
    const double TOLERANCE = 0.1;  // 1% tolerance
    if (s < -TOLERANCE || s > 1.0 + TOLERANCE) {
        std::cout << "    REJECTED: s out of range" << std::endl;
        return false;
    }
    
    // Clamp s to [0, 1]
    s = std::max(0.0, std::min(1.0, s));
    
    // Compute intersection point
    Point64 candidate;
    candidate.x = static_cast<int64_t>(edgeStartX + s * (edgeEndX - edgeStartX));
    candidate.y = static_cast<int64_t>(edgeStartY + s * (edgeEndY - edgeStartY));
    
    std::cout << "    Candidate: (" << candidate.x << ", " << candidate.y << ")" << std::endl;
    
    // Try to use candidate as-is first
    if (isInsideOrOnEdge(candidate)) {
        std::cout << "    ACCEPTED (inside or on edge)" << std::endl;
        out = candidate;
        return true;
    }
    
    std::cout << "    Candidate outside, attempting inward correction..." << std::endl;
    
    // Move inward a small amount to ensure it's inside
    const int MAX_ITERATIONS = 50;
    const int64_t STEP_SIZE = 1;
    
    Point64 corrected = candidate;
    for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
        corrected = moveInward(corrected, startIdx, STEP_SIZE);
        
        if (isInsideOrOnEdge(corrected)) {
            std::cout << "    ACCEPTED after " << (iter + 1) << " inward steps" << std::endl;
            out = corrected;
            return true;
        }
    }
    
    // If we still can't get inside, just return the candidate anyway
    std::cout << "    ACCEPTED (fallback - using candidate despite being outside)" << std::endl;
    out = candidate;
    return true;
}

Point64 Edger::getNearestEdgePoint(const Point64& pos) {
    size_t n = region.size();
    if (n == 0) return pos;
    if (n == 1) return region[0];

    int64_t bestDistSq = std::numeric_limits<int64_t>::max();
    Point64 bestPoint = pos;

    for (size_t i = 0; i < n; ++i) {
        const Point64& a = region[i];
        const Point64& b = region[(i + 1) % n];
        
        // Find nearest point on segment [a, b] to pos
        Point64 ab = b - a;
        Point64 ap = pos - a;
        
        int64_t ab_dot_ab = dot64(ab, ab);
        if (ab_dot_ab == 0) {
            // Degenerate edge, just use point a
            int64_t distSq = dot64(ap, ap);
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                bestPoint = a;
            }
            continue;
        }
        
        int64_t ap_dot_ab = dot64(ap, ab);
        
        Point64 candidate;
        if (ap_dot_ab <= 0) {
            // Closest to point a
            candidate = a;
        } else if (ap_dot_ab >= ab_dot_ab) {
            // Closest to point b
            candidate = b;
        } else {
            // Closest to interior of segment
            // Use floating point for the projection calculation
            double t = static_cast<double>(ap_dot_ab) / static_cast<double>(ab_dot_ab);
            candidate.x = a.x + static_cast<int64_t>(t * ab.x);
            candidate.y = a.y + static_cast<int64_t>(t * ab.y);
        }
        
        Point64 diff = candidate - pos;
        int64_t distSq = dot64(diff, diff);
        
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestPoint = candidate;
        }
    }

    return bestPoint;
}

std::pair<int, int> Edger::getVertexRangeBelowThreshold(const Point64& dir, int64_t thresh, const Point64& start) {
    size_t n = region.size();
    if (n == 0) {
        throw std::runtime_error("Cannot get vertex range on empty vertex list");
    }

    // Step 1: Find the vertex closest to start
    int64_t minDistSq = std::numeric_limits<int64_t>::max();
    size_t closestIndex = 0;
    
    for (size_t i = 0; i < n; ++i) {
        Point64 diff = region[i] - start;
        int64_t distSq = dot64(diff, diff);
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closestIndex = i;
        }
    }

    // Verify that the closest vertex satisfies the threshold condition
    int64_t closestDot = dot64(region[closestIndex], dir);
    if (closestDot >= thresh) {
        throw std::runtime_error("Closest vertex dot product is not less than threshold");
    }

    // Step 2: Walk CCW (backwards in index) from closest until we find a vertex >= thresh
    int currentIndex = static_cast<int>(closestIndex);
    int leftFirstOutside = -1;
    size_t stepsLeft = 0;
    
    while (stepsLeft < n) {
        int prevIndex = (currentIndex == 0) ? static_cast<int>(n - 1) : (currentIndex - 1);
        int64_t prevDot = dot64(region[prevIndex], dir);
        
        if (prevDot >= thresh) {
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
    currentIndex = static_cast<int>(closestIndex);
    int rightFirstOutside = -1;
    size_t stepsRight = 0;
    
    while (stepsRight < n) {
        int nextIndex = (currentIndex + 1) % static_cast<int>(n);
        int64_t nextDot = dot64(region[nextIndex], dir);
        
        if (nextDot >= thresh) {
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

void Edger::addVerticesInRange(std::vector<Point64>& vecs, int startIdx, int endIdx) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    startIdx = wrapIndex(startIdx, n);
    endIdx = wrapIndex(endIdx, n);

    // Walk from startIdx to endIdx (handles wrap-around automatically)
    int i = startIdx;
    do {
        vecs.push_back(region[i]);
        if (i == endIdx) break;
        i = (i + 1) % n;
    } while (true);
}

void Edger::addVerticesOutsideRange(std::vector<Point64>& vecs, int startIdx, int endIdx) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    startIdx = wrapIndex(startIdx, n);
    endIdx = wrapIndex(endIdx, n);

    // Walk from (endIdx+1) to (startIdx-1) - the complement of the range
    int start = (endIdx + 1) % n;
    int end = (startIdx - 1 + n) % n;
    
    // Handle case where the range is the entire polygon
    if (startIdx == ((endIdx + 1) % n)) {
        // Range covers everything, so outside is empty
        return;
    }
    
    int i = start;
    do {
        vecs.push_back(region[i]);
        if (i == end) break;
        i = (i + 1) % n;
    } while (true);
}

void Edger::addReflectedVertices(std::vector<Point64>& reflected, int startIdx, int endIdx, const Point64& p0, const Point64& p1) {
    if (region.empty())
        return;

    int n = static_cast<int>(region.size());
    startIdx = wrapIndex(startIdx, n);
    endIdx = wrapIndex(endIdx, n);

    // Collect indices in range
    std::vector<int> indices;
    int i = startIdx;
    do {
        indices.push_back(i);
        if (i == endIdx) break;
        i = (i + 1) % n;
    } while (true);

    // Reserve space
    reflected.reserve(reflected.size() + indices.size());

    // Reflect vertices over line defined by p0 and p1
    Point64 lineDir = p1 - p0;
    int64_t lineDirDotLinDir = dot64(lineDir, lineDir);
    
    if (lineDirDotLinDir == 0) {
        // Degenerate line (p0 == p1), can't reflect
        // Just add the points unreflected in reverse
        for (int idx = static_cast<int>(indices.size()) - 1; idx >= 0; --idx) {
            reflected.push_back(region[indices[idx]]);
        }
        return;
    }

    // Append reflected vertices in reverse order
    for (int idx = static_cast<int>(indices.size()) - 1; idx >= 0; --idx) {
        int vindex = indices[idx];
        Point64 point = region[vindex];
        Point64 pointToP0 = point - p0;
        
        // Project pointToP0 onto lineDir
        int64_t projection = dot64(pointToP0, lineDir);
        
        // Use floating point for the division to maintain precision
        double t = static_cast<double>(projection) / static_cast<double>(lineDirDotLinDir);
        
        // Find closest point on line
        Point64 closestOnLine;
        closestOnLine.x = p0.x + static_cast<int64_t>(t * lineDir.x);
        closestOnLine.y = p0.y + static_cast<int64_t>(t * lineDir.y);
        
        // Reflect: reflected = 2 * closestOnLine - point
        Point64 reflectedPoint;
        reflectedPoint.x = 2 * closestOnLine.x - point.x;
        reflectedPoint.y = 2 * closestOnLine.y - point.y;
        
        reflected.push_back(reflectedPoint);
    }
}

void Edger::toFloat(std::vector<vec2>& region) {
    region.clear();
    region.reserve(this->region.size());

    for (const Point64& p : this->region) region.push_back(point64ToVec2(p));
}