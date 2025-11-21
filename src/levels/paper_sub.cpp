#include "levels/levels.h"

// ------------------------------------------------------------
// PaperMesh
// ------------------------------------------------------------

Paper::PaperMesh::PaperMesh(const std::vector<Point64>& verts, Mesh* mesh) : DyMesh(verts, mesh), mesh(nullptr) {
    std::vector<float> data; 
    toData(data);
    this->mesh = new Mesh(data);
}

Paper::PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
}

Paper::PaperMesh::PaperMesh(const PaperMesh& other) : DyMesh(other.region, other.data), mesh(nullptr) {
    std::vector<float> data;
    toData(data);
    mesh = new Mesh(data);
}

Paper::PaperMesh::PaperMesh(PaperMesh&& other) noexcept : DyMesh(std::move(other.region), std::move(other.data)), mesh(other.mesh) {
    other.mesh = nullptr;
}

Paper::PaperMesh& Paper::PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    PaperMesh temp(other);
    
    delete mesh;
    mesh = temp.mesh;
    region = std::move(temp.region);
    data = std::move(temp.data);
    temp.mesh = nullptr;
    
    return *this;
}

// Move assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    
    region = std::move(other.region);
    data = std::move(other.data);
    mesh = other.mesh;
    other.mesh = nullptr;
    
    return *this;
}

void Paper::PaperMesh::regenerateMesh() {
    Mesh* oldPaperMesh = mesh;
    std::vector<float> newMeshData;
    toData(newMeshData);
    mesh = new Mesh(newMeshData);
    delete oldPaperMesh;
}

// ------------------------------------------------------------
// Fold
// ------------------------------------------------------------

// Fold constructor refactored to use Point64 and p0/p1 line representation
// Fold constructor with comprehensive debugging
Paper::Fold::Fold(PaperMesh* paperMesh, const Point64& crease0, const Point64& crease1, const Point64& edgeIntersectPaper, const Point64& start, int side) :
    underside(nullptr),
    cover(nullptr),
    holds(),
    start(start),
    side(side)
{
    Edger edger(paperMesh->region);
    
    std::cout << "\n=== FOLD CONSTRUCTOR DEBUG ===" << std::endl;
    
    // Step 1: Find where crease line intersects the paper edges
    // The crease is an infinite line passing through crease0 and crease1
    std::vector<std::pair<int, Point64>> intersections;
    
    int n = paperMesh->region.size();
    for (int i = 0; i < n; i++) {
        Point64 intersection;
        if (edger.getEdgeIntersection(i, crease0, crease1, intersection)) {
            intersections.push_back({i, intersection});
            std::cout << "Crease intersects edge " << i << " at (" << intersection.x << ", " << intersection.y << ")" << std::endl;
        }
    }
    
    if (intersections.size() != 2) {
        std::cout << "ERROR: Expected 2 intersections, found " << intersections.size() << std::endl;
        throw std::runtime_error("Fold crease must intersect exactly 2 edges");
    }
    
    Point64 foldStart = intersections[0].second;
    Point64 foldEnd = intersections[1].second;
    int startEdgeIdx = intersections[0].first;
    int endEdgeIdx = intersections[1].first;
    
    // Step 2: Determine which vertices are on the "fold side" of the crease
    // Use the perpendicular to the crease line
    Point64 creaseLine = crease1 - crease0;
    Point64 creaseNormal = {-creaseLine.y, creaseLine.x};  // 90° CCW
    int64_t thresh = dot64(crease0, creaseNormal);
    
    std::cout << "Crease threshold: " << thresh << std::endl;
    
    // Check each vertex to see which side of the crease it's on
    std::vector<int> verticesOnFoldSide;
    for (int i = 0; i < n; i++) {
        int64_t vertDot = dot64(paperMesh->region[i], creaseNormal);
        std::cout << "Vertex " << i << " dot: " << vertDot << " (thresh: " << thresh << ")" << std::endl;
        
        if (vertDot < thresh) {
            verticesOnFoldSide.push_back(i);
            std::cout << "  -> ON FOLD SIDE" << std::endl;
        }
    }
    
    if (verticesOnFoldSide.empty()) {
        throw std::runtime_error("No vertices on fold side - invalid fold");
    }
    
    std::cout << "Vertices on fold side: ";
    for (int idx : verticesOnFoldSide) std::cout << idx << " ";
    std::cout << std::endl;
    
    // Step 3: Build the "cut" region (the part being folded over)
    // This should be: foldStart -> fold-side vertices (in order) -> foldEnd
    std::vector<Point64> cutVerts = { foldStart };
    
    // Add vertices in order around the polygon
    // We need to walk from the first fold-side vertex to the last
    if (verticesOnFoldSide.size() > 0) {
        int firstIdx = verticesOnFoldSide.front();
        int lastIdx = verticesOnFoldSide.back();
        edger.addVerticesInRange(cutVerts, firstIdx, lastIdx);
    }
    
    cutVerts.push_back(foldEnd);
    
    std::cout << "Cut region has " << cutVerts.size() << " vertices" << std::endl;
    
    // Step 4: Copy UV data from the paper for this region
    DyMesh cut = DyMesh(cutVerts);
    bool check = cut.copy(*paperMesh);
    if (!check) {
        throw std::runtime_error("Failed to copy negative-cut UVs");
    }
    
    // Step 5: Mirror the cut region over the crease to create the "cover"
    cover = cut.mirror(crease0, crease1);
    std::cout << "Cover created with " << cover->region.size() << " vertices" << std::endl;
    
    // Step 6: Create the "underside" - the reflected vertices that will be hidden
    std::vector<Point64> undersideVerts = { foldStart };
    
    if (verticesOnFoldSide.size() > 0) {
        int firstIdx = verticesOnFoldSide.front();
        int lastIdx = verticesOnFoldSide.back();
        edger.addReflectedVertices(undersideVerts, firstIdx, lastIdx, crease0, crease1);
    }
    
    undersideVerts.push_back(foldEnd);
    
    std::cout << "Underside has " << undersideVerts.size() << " vertices" << std::endl;
    
    underside = new DyMesh(undersideVerts);
    check = underside->copyIntersection(*paperMesh);
    if (!check) {
        std::cout << "Warning: Failed to copy underlayer intersection" << std::endl;
    }
    
    std::cout << "=== FOLD CONSTRUCTOR SUCCESS ===\n" << std::endl;
}

// Rule of 5 implementations remain the same
Paper::Fold::~Fold() {
    delete underside; underside = nullptr;
    delete cover; cover = nullptr;
}

Paper::Fold::Fold(const Fold& other) :
    underside(other.underside ? new DyMesh(*other.underside) : nullptr),
    cover(other.cover ? new DyMesh(*other.cover) : nullptr),
    holds(other.holds),
    start(other.start),
    side(other.side)
{}

Paper::Fold::Fold(Fold&& other) noexcept :
    underside(other.underside),
    cover(other.cover),
    holds(std::move(other.holds)),
    start(std::move(other.start)),
    side(other.side)
{
    other.underside = nullptr;
    other.cover = nullptr;
}

Paper::Fold& Paper::Fold::operator=(const Fold& other) {
    if (this == &other) return *this;
    
    Fold temp(other);
    
    delete underside;
    delete cover;
    
    underside = temp.underside;
    cover = temp.cover;
    holds = std::move(temp.holds);
    start = temp.start;
    side = temp.side;
    
    temp.underside = nullptr;
    temp.cover = nullptr;
    
    return *this;
}

Paper::Fold& Paper::Fold::operator=(Fold&& other) noexcept {
    if (this == &other) return *this;
    
    delete underside;
    delete cover;
    
    underside = other.underside;
    cover = other.cover;
    holds = std::move(other.holds);
    start = std::move(other.start);
    side = other.side;
    
    other.underside = nullptr;
    other.cover = nullptr;
    
    return *this;
}