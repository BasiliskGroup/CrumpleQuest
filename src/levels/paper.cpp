#include "levels/levels.h"

Paper::PaperMesh::PaperMesh(const std::vector<vec2> region, const std::vector<Vert>& data) : DyMesh(region), mesh(nullptr) {
    std::vector<float> flatData; 
    Paper::flattenVertices(data, flatData);
    mesh = new Mesh(flatData);
}

Paper::PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
}

// Copy constructor
Paper::PaperMesh::PaperMesh(const PaperMesh& other) : DyMesh(other.region), mesh(nullptr) {
    if (other.mesh) {
        std::vector<float> data;
        toData(data);
        mesh = new Mesh(data);
    }
}

// Move constructor
Paper::PaperMesh::PaperMesh(PaperMesh&& other) noexcept : DyMesh(std::move(other.region)), mesh(other.mesh) {
    other.mesh = nullptr;
}

// Copy assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    PaperMesh temp(other);
    
    delete mesh;
    mesh = temp.mesh;
    region = std::move(temp.region);
    temp.mesh = nullptr;
    
    return *this;
}

// Move assignment
Paper::PaperMesh& Paper::PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    
    region = std::move(other.region);
    mesh = other.mesh;
    other.mesh = nullptr;
    
    return *this;
}

// ==================== Paper Implementation ====================

Paper::Paper() 
    : curSide(0), isOpen(false), activeFold(-1)
{
    sides = { nullptr, nullptr };
    paperMeshes = { nullptr, nullptr };
}

Paper::Paper(Mesh* mesh, const std::vector<vec2>& edgeVerts) : 
    curSide(0), 
    isOpen(false),
    activeFold(-1),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    const auto& meshVerts = mesh->getVertices();

    std::vector<Vert> verts;
    for (uint i = 0; i < meshVerts.size(); i += 5) {
        // no i + 2 since that is z and we're flat
        verts.push_back({ { meshVerts[i + 0], meshVerts[i + 1] }, { meshVerts[i + 2], meshVerts[i + 3] } });
    }
    paperMeshes.first = new PaperMesh(edgeVerts, verts);

    // reverse x direction for other side of paper
    for (Vert& v : verts) {
        v.pos.x = -v.pos.x;
        // TODO check if we need to invert UVs
    }
    paperMeshes.second = new PaperMesh(edgeVerts, verts);

    initFolds(edgeVerts);
}

Paper::Paper(SingleSide* sideA, SingleSide* sideB, short startSide, bool isOpen) 
    : isOpen(isOpen), curSide(startSide), activeFold(-1) 
{
    sides = { sideA, sideB };
    paperMeshes = { nullptr, nullptr };
}

Paper::Paper(const Paper& other)
    : curSide(other.curSide), 
      isOpen(other.isOpen),
      folds(other.folds),
      activeFold(other.activeFold),
      sides({ nullptr, nullptr }),
      paperMeshes({ nullptr, nullptr })
{
    try {
        // Deep copy sides
        if (other.sides.first)
            sides.first = new SingleSide(*other.sides.first);
            
        if (other.sides.second)
            sides.second = new SingleSide(*other.sides.second);
        
        // Deep copy paperMeshes
        if (other.paperMeshes.first)
            paperMeshes.first = new PaperMesh(*other.paperMeshes.first);
            
        if (other.paperMeshes.second)
            paperMeshes.second = new PaperMesh(*other.paperMeshes.second);
    }
    catch (...) {
        // Clean up any partially constructed objects
        delete sides.first;
        delete sides.second;
        delete paperMeshes.first;
        delete paperMeshes.second;
        throw;
    }
}

// Move constructor
Paper::Paper(Paper&& other) noexcept
    : curSide(other.curSide),
      isOpen(other.isOpen),
      folds(std::move(other.folds)),
      activeFold(other.activeFold),
      sides(std::move(other.sides)),
      paperMeshes(std::move(other.paperMeshes))
{
    // Clear other
    other.sides = { nullptr, nullptr };
    other.paperMeshes = { nullptr, nullptr };
    other.activeFold = -1;
}

Paper::~Paper() {
    clear();
}

// Copy assignment - REMOVED noexcept since it can throw
Paper& Paper::operator=(const Paper& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for strong exception safety
    Paper temp(other);
    
    // Swap with temp
    std::swap(sides, temp.sides);
    std::swap(paperMeshes, temp.paperMeshes);
    std::swap(curSide, temp.curSide);
    std::swap(isOpen, temp.isOpen);
    std::swap(folds, temp.folds);
    std::swap(activeFold, temp.activeFold);
    
    // temp's destructor will clean up our old resources
    return *this;
}

// Move assignment
Paper& Paper::operator=(Paper&& other) noexcept {
    if (this == &other) return *this;
    
    clear();

    // Transfer ownership
    sides = std::move(other.sides);
    paperMeshes = std::move(other.paperMeshes);
    curSide = other.curSide;
    isOpen = other.isOpen;
    folds = std::move(other.folds);
    activeFold = other.activeFold;

    // Clear other
    other.sides = { nullptr, nullptr };
    other.paperMeshes = { nullptr, nullptr };
    other.activeFold = -1;

    return *this;
}

void Paper::flip() {
    curSide = curSide == 0 ? 1 : 0;
}

void Paper::open() {
    isOpen = true;
}

void Paper::clear() {
    delete sides.first;
    delete sides.second;
    sides = { nullptr, nullptr };
    
    delete paperMeshes.first;
    delete paperMeshes.second;
    paperMeshes = { nullptr, nullptr };
    
    isOpen = true;
}

Paper::Fold::Fold(const std::vector<vec2>& verts, vec2 crease, int layer, int side) 
    : DyMesh(verts), crease(crease), layer(layer), side(side)
{
    // find indices
    std::vector<uint> inds;
    Navmesh::earcut({verts}, inds);
    
    // create triangles
    for (uint i = 0; i < inds.size(); i += 3) {
        data.push_back(Tri({ 
            verts[inds[i + 0]],
            verts[inds[i + 1]],
            verts[inds[i + 2]]
        }));
        std::cout << "<" << verts[inds[i + 0]].x << ", " << verts[inds[i + 0]].y << ">";
        std::cout << "<" << verts[inds[i + 1]].x << ", " << verts[inds[i + 1]].y << ">";
        std::cout << "<" << verts[inds[i + 2]].x << ", " << verts[inds[i + 1]].y << ">";
        std::cout << std::endl;
    }
}

bool Paper::Fold::contains(const vec2& pos) {
    for (const Tri& tri : data) {
        if (tri.contains(pos)) return true;
    }
    return false;
}

void Paper::initFolds(const std::vector<vec2>& edgeVerts) {
    if (paperMeshes.first == nullptr) {
        throw std::runtime_error("Cannot create fold with no paper mesh");
    }
    folds.push_back(Fold(edgeVerts, {0, 0}, 0));
}

Mesh* Paper::getMesh() { 
    PaperMesh* curMesh = curSide ? paperMeshes.second : paperMeshes.first; 
    if (curMesh == nullptr) return nullptr;
    return curMesh->mesh;
}

void Paper::activateFold(const vec2& start) {
    activeFold = -1;

    // we push_back like a stack so the first fold we find is the top
    for (int i = static_cast<int>(folds.size()) - 1; i >= 0; i--) {
        if (folds[i].side != curSide) continue;

        if (folds[i].contains(start)) {
            activeFold = i;
            return;
        }
    }
}

void Paper::deactivateFold() {
    activeFold = -1;
}

void Paper::fold(const vec2& start, const vec2& end) {
    if (activeFold == -1 || glm::length2(start - end) < EPSILON) return;
    
    // 1. locate triangle that we are on
    Fold& fold = folds[activeFold];
    uint trindex = -1;
    for (uint i = 0; i < fold.data.size(); i++) {
        if (fold.data[i].contains(start)) {
            trindex = i;
            break;
        }
    }
    if (trindex == -1) {
        std::cout << "could not start identify triangle" << std::endl;
        return;
    }
    Tri& clickedTri = fold.data[trindex];
    PaperMesh* paperMesh = curSide == 0 ? paperMeshes.first : paperMeshes.second;
    
    // 2. get crease line
    vec2 dx = end - start;
    
    // get intersection and locality data
    vec2 edgeIntersectFold = fold.getNearestEdgeIntersection(start, -dx);
    vec2 nearEdgePointFold = fold.getNearestEdgePoint(start);
    vec2 edgeIntersectPaper = paperMesh->getNearestEdgeIntersection(start, -dx);
    vec2 nearEdgePointPaper = paperMesh->getNearestEdgePoint(start);
    vec2 deepestPointOnTri = clickedTri.leastDot(dx);

    // variable to modify "start" position of fold, drop and replace 
    vec2 foldStart = nearEdgePointPaper;

    dx = end - foldStart;
    vec2 creaseDir = { dx.y, -dx.x };

    new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("paper"), .scale={0.25, 0.25}, .position=edgeIntersectPaper });
    
    vec2 midPoint = 0.5f * (end + foldStart);
    float midDot = glm::dot(midPoint, dx);
    
    // make a new fold in the paper
    if (glm::dot(edgeIntersectPaper - start, nearEdgePointPaper - start) > 0) {
        std::cout << "fold" << std::endl;

        vec2 searchStart = clickedTri.leastDot(dx);
        auto indexBounds = paperMesh->getVertexRangeBelowThreshold(dx, midDot, searchStart);
        
        // Calculate the actual edge indices
        int leftEdgeIndex = indexBounds.first - 1;
        int rightEdgeIndex = indexBounds.second;
        
        // Print the vertices that form these edges
        int leftStart = leftEdgeIndex;
        int leftEnd = indexBounds.first;
        if (leftStart < 0) leftStart += paperMesh->region.size();
        if (leftEnd < 0) leftEnd += paperMesh->region.size();
        
        int rightStart = rightEdgeIndex % paperMesh->region.size();
        int rightEnd = (rightEdgeIndex + 1) % paperMesh->region.size();
        
        vec2 foldStart, foldEnd;
        bool leftCheck = paperMesh->getEdgeIntersection(indexBounds.first - 1, midPoint, creaseDir, foldStart);
        bool rightCheck = paperMesh->getEdgeIntersection(indexBounds.second, midPoint, creaseDir, foldEnd);
        
        bool check = leftCheck & rightCheck;
        
        if (!check) throw std::runtime_error("Fold crease could not find intersection");

        new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .scale={0.25, 0.25}, .position=foldStart });
        new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .scale={0.25, 0.25}, .position=foldEnd });

        // create new fold
        std::vector<vec2> newFoldVerts = { foldStart, foldEnd };
        paperMesh->reflectVerticesOverLine(newFoldVerts, indexBounds.first, indexBounds.second, midPoint, creaseDir);
        for (vec2& v : newFoldVerts) v.y *= -1;
        std::vector<uint> newFoldIndices;
        Navmesh::earcut({newFoldVerts}, newFoldIndices);
        std::vector<float> newFoldData;
        Navmesh::convertToMesh({newFoldVerts}, newFoldIndices, newFoldData);
        std::string meshName = std::to_string(midPoint.x) + " " + std::to_string(midPoint.y);
        game->addMesh(meshName, new Mesh(newFoldData));
        Node2D* tempNode = new Node2D(game->getScene(), { .mesh=game->getMesh(meshName), .material=game->getMaterial("box"), .position={0, 3} });

        // modify the mesh to accommodate new fold
        Mesh* oldPaperMesh = paperMesh->mesh;
        std::vector<vec2> newMeshVerts = { foldStart, foldEnd };
        paperMesh->getUnreflectedVertices(newMeshVerts, indexBounds.first, indexBounds.second);
        for (vec2& v : newMeshVerts) v.y *= -1;
        std::vector<uint> newMeshIndices;
        Navmesh::earcut({newMeshVerts}, newMeshIndices);
        std::vector<float> newMeshData;
        Navmesh::convertToMesh({newMeshVerts}, newMeshIndices, newMeshData);
        paperMesh->mesh = new Mesh(newMeshData);
        delete oldPaperMesh;

        // TODO delete old mesh

    } else {
        std::cout << "unfold" << std::endl;
    }
    
    // TODO add in an unfold function
    // TODO add pulling a fold forward
    
    // 4. create fold polygon
    // 5. clip polygon to remove folded over section
    // 6. earcut remaining paper and remap uvs
    // 7. earcut fold and mirror uvs
    // 8. clip the back (should be no uv changes)
    // 9. lock folds if they are folded back or covered. 
}