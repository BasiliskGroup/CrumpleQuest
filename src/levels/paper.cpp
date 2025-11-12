#include "levels/levels.h"

Paper::PaperMesh::PaperMesh(const std::vector<vec2> verts, Mesh* mesh) : DyMesh(verts, mesh), mesh(nullptr) {
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

Paper::Paper() : 
    curSide(0), 
    isOpen(false),
    activeFold(-1),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{}

Paper::Paper(Mesh* mesh, const std::vector<vec2>& region) : 
    curSide(0), 
    isOpen(false),
    activeFold(-1),
    sides(nullptr, nullptr), 
    paperMeshes(nullptr, nullptr) 
{
    paperMeshes.first = new PaperMesh(region, mesh);
    paperMeshes.second = new PaperMesh(region, mesh);

    initFolds(region);
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

    // Temporary
    for (uint i = 0; i < regionNodes.size(); i++) {
        delete regionNodes[i];
    }
    regionNodes.clear();
}

Paper::Fold::Fold(const std::vector<vec2>& verts, int side) 
    : DyMesh(verts), side(side)
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
    folds.push_back(Fold(edgeVerts));
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
    uint trindex = fold.getTrindex(start);
    if (trindex == -1) {
        std::cout << "could not start identify triangle" << std::endl;
        return;
    }
    PaperMesh* paperMesh = curSide == 0 ? paperMeshes.first : paperMeshes.second;
    Tri& clickedTri = paperMesh->data[trindex];
    
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

    new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("paper"), .scale={0.25, 0.25}, .position=foldStart });
    
    vec2 midPoint = 0.5f * (end + foldStart);
    float midDot = glm::dot(midPoint, dx);
    
    // make a new fold in the paper
    if (true || glm::dot(edgeIntersectPaper - start, nearEdgePointPaper - start) > 0) {
        std::cout << "fold" << std::endl;

        uint trindex = paperMesh->getTrindex(edgeIntersectPaper);
        vec2 searchStart = paperMesh->data[trindex].leastDot(dx);
        auto indexBounds = paperMesh->getVertexRangeBelowThreshold(dx, midDot, searchStart);
        
        // Calculate the actual edge indices
        int leftEdgeIndex = indexBounds.first - 1;
        int rightEdgeIndex = indexBounds.second;
        
        vec2 foldStart, foldEnd;
        bool leftCheck = paperMesh->getEdgeIntersection(indexBounds.first - 1, midPoint, creaseDir, foldStart);
        bool rightCheck = paperMesh->getEdgeIntersection(indexBounds.second, midPoint, creaseDir, foldEnd);
        
        bool check = leftCheck & rightCheck;
        
        if (!check) throw std::runtime_error("Fold crease could not find intersection");

        new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position=foldStart, .scale={0.25, 0.25} });
        new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .position=foldEnd, .scale={0.25, 0.25} });

        std::cout << "New Fold Verts" << std::endl;

        // create new fold
        std::vector<vec2> newFoldVerts = { foldStart };
        paperMesh->addVertexRange(newFoldVerts, indexBounds);
        newFoldVerts.push_back(foldEnd);

        paperMesh->reflectVerticesOverLine(newFoldVerts, indexBounds.first, indexBounds.second, midPoint, creaseDir);

        std::reverse(newFoldVerts.begin(), newFoldVerts.end());
        Fold newFold = Fold(newFoldVerts);

        std::vector<float> newFoldData;
        newFold.toData(newFoldData);
        
        std::string meshName = std::to_string(midPoint.x) + " " + std::to_string(midPoint.y);
        game->addMesh(meshName, new Mesh(newFoldData));
        Node2D* tempNode = new Node2D(game->getScene(), { .mesh=game->getMesh(meshName), .material=game->getMaterial("box") });
        tempNode->setLayer(-0.9);

        // modify the mesh to accommodate new fold
        paperMesh->cut(newFold);
        Mesh* oldPaperMesh = paperMesh->mesh;
        std::vector<float> newMeshData;
        paperMesh->toData(newMeshData);
        paperMesh->mesh = new Mesh(newMeshData);
        delete oldPaperMesh;

        // Display region for debug
        for (uint i = 0; i < regionNodes.size(); i++) {
            delete regionNodes[i];
        }
        regionNodes.clear();

        for (auto& r : paperMesh->region) {
            Node2D* n = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position=r, .scale={0.1, 0.1} });
            n->setLayer(0.9);
            regionNodes.push_back(n);
        }

    } else {
        std::cout << "unfold" << std::endl;
    }
    
    // TODO add in an unfold function
    // TODO add pulling a fold forward
}