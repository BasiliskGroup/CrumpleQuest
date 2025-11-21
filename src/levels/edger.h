#ifndef EDGER_H
#define EDGER_H

#include "util/includes.h"
#include "util/clipper_helper.h"
#include "util/maths.h"

struct Edger {
    std::vector<Point64> region; // CCW wound polygon

    Edger(const std::vector<Point64>& region);
    Edger(const std::vector<vec2>& region);

    // Intersection functions - return false if no valid intersection found
    bool getNearestEdgeIntersection(const Point64& p0, const Point64& p1, Point64& out);
    bool getEdgeIntersection(int edgeStartIndex, const Point64& p0, const Point64& p1, Point64& out);

    // Nearest point (always succeeds for non-empty polygons)
    Point64 getNearestEdgePoint(const Point64& pos);
    
    // Vertex range finding
    std::pair<int, int> getVertexRangeBelowThreshold(const Point64& dir, int64_t thresh, const Point64& start);
    
    // Vertex manipulation (pure int64_t)
    void addVerticesInRange(std::vector<Point64>& vecs, int startIdx, int endIdx);
    void addVerticesOutsideRange(std::vector<Point64>& vecs, int startIdx, int endIdx);
    void addReflectedVertices(std::vector<Point64>& reflected, int startIdx, int endIdx, const Point64& p0, const Point64& p1);

    void toFloat(std::vector<vec2>& region);

private:
    // Helper to test if point is inside or on edge (CCW polygon)
    bool isInsideOrOnEdge(const Point64& point) const;
    
    // Helper to move point inward perpendicular to an edge
    Point64 moveInward(const Point64& point, size_t edgeIndex, int64_t stepSize) const;
};

// Helper functions
inline Point64 vec2ToPoint64(const vec2& v) { return { 
    static_cast<int64_t>(v.x * CLIPPER_SCALE), 
    static_cast<int64_t>(v.y * CLIPPER_SCALE) 
}; }

inline vec2 point64ToVec2(const Point64& p) { return { 
    p.x / (float) CLIPPER_SCALE, 
    p.y / (float) CLIPPER_SCALE 
}; }

inline int64_t dot64(const Point64& a, const Point64& b) { 
    return a.x * b.x + a.y * b.y; 
}

inline int64_t length264(const Point64& p) {
    return p.x * p.x + p.y * p.y;
}

inline int wrapIndex(int idx, int n) {
    idx = idx % n;
    if (idx < 0) idx += n;
    return idx;
}

#endif