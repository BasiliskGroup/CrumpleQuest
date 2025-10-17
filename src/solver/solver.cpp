#include "solver/physics.h"


Solver::Solver() : forces(nullptr), bodies(nullptr) {
    // set default params
    gravity = -10.0f;
    iterations = 10;
    beta = 100000.0f;
    alpha = 0.99f;
    gamma = 0.99f;

    // create Tables
    forceTable = new ForceTable(1024);
    bodyTable = new BodyTable(512);
    meshFlat = new MeshFlat(256, 16);
}

Solver::~Solver() {
    clear();
}

/**
 * @brief Completes a single step for the solver
 * 
 * @param dt time that has passed since last step
 */
void Solver::step(float dt) {
    auto beforeStep = timeNow();
    compactBodies();
    printDurationUS(beforeStep, timeNow(), "Body Compact:\t\t");

    auto beforeTransform = timeNow();
    bodyTable->computeTransforms();
    printDurationUS(beforeTransform, timeNow(), "Body Transform:\t\t");

    // NOTE bodies are compact after this point

    auto beforeBroad = timeNow();
    sphericalCollision(); // broad collision TODO replace with BVH
    std::cout << "Broad Pairs:\t\t" << collisionPairs.size() << std::endl;
    printDurationUS(beforeBroad, timeNow(), "Broad Collision:\t");

    auto beforeNarrow = timeNow();
    narrowCollision(); // -> uncompacts manifolds
    printDurationUS(beforeNarrow, timeNow(), "Narrow Collision:\t");

    // warmstart forces -> uncompacts forces
    auto beforeCompact = timeNow();
    compactForces();
    printDurationUS(beforeCompact, timeNow(), "Force Compact:\t\t");

    // NOTE bodies and forces are compact after this point

    auto beforeManifoldWarm = timeNow();
    warmstartManifolds();
    printDurationUS(beforeManifoldWarm, timeNow(), "Manifold Warm:\t\t");

    auto beforeForceWarm = timeNow();
    void warmstartForces();
    printDurationUS(beforeForceWarm, timeNow(), "Force Warm:\t\t");

    auto beforeBodyWarm = timeNow();
    warmstartBodies(dt);
    printDurationUS(beforeBodyWarm, timeNow(), "Body Warm:\t\t");

    auto beforeMainPreload = timeNow();
    mainloopPreload();
    printDurationUS(beforeMainPreload, timeNow(), "Preload:\t\t");

    print("-----------------------------------------");

    // main solver loop
    auto beforeMain = timeNow();
    for (ushort iter = 0; iter < iterations; iter++) {
        auto beforePrimal = timeNow();
        primalUpdate(dt);
        printPrimalDuration(beforePrimal, timeNow());

        auto beforeDual = timeNow();
        dualUpdate(dt);
        printDualDuration(beforeDual, timeNow());
    }
    printDurationUS(beforeMain, timeNow(), "Main Loop:\t\t");

    auto beforeVel = timeNow();
    updateVelocities(dt);
    printDurationUS(beforeVel, timeNow(), "Velocities:\t\t");

    print("------------------------------------------");
    printDurationUS(beforeStep, timeNow(), "Total: ");
    print("");
}

void Solver::compactBodies() {
    bodyTable->compact();

    // route forces back to their correct bodies after standard compact
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& toDelete = forceTable->getToDelete();
    auto& inverseForceMap = bodyTable->getInverseForceMap();

    for (uint i = 0; i < forceTable->getSize(); i++) {
        // will index out of bounds
        if (toDelete[i] == true) {
            continue;
        }
        bodyIndices[i] = inverseForceMap[bodyIndices[i]];
    }
}

void Solver::compactForces() {
    forceTable->compact();
}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    auto& pos = bodyTable->getPos();
    auto& radii = bodyTable->getRadius();

    // clear contact pairs from last frame
    collisionPairs.clear();

    uint bodyCount = 0;
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        bodyCount++;
    }

    // load body-force relations
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->precomputeRelations();
    }

    vec2 dpos;
    float dy;
    float radsum;
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext()) {
        for (Rigid* bodyB = bodyA->getNext(); bodyB != nullptr; bodyB = bodyB->getNext()) {
            uint i = bodyA->getIndex();
            uint j = bodyB->getIndex();

            // ignore collision flag
            if (bodyA->constrainedTo(j) == MANIFOLD) {
                continue;
            }

            dpos = pos[i] - pos[j];
            radsum = radii[i] + radii[j];
            if (radsum * radsum > glm::length2(dpos)) {
                collisionPairs.emplace_back(i, j);
            }
        }
    }
}

void Solver::narrowCollision() {
    auto& manifoldNormals = getManifoldTable()->getNormal();
    auto& forcePointers = forceTable->getForces();
    auto& bodyPointers = bodyTable->getBodies();
    auto& specialIndices = forceTable->getSpecial();
    auto& types = forceTable->getType();

    // reserve space to perform all collisions 
    uint forceIndex, manifoldIndex;
    reserveForcesForCollision(forceIndex, manifoldIndex);

    int count = 0; // TODO debug variable

    ColliderRow a, b;
    CollisionPair collisionPair;
    for (const auto& pair : collisionPairs) {
        uint rowA = pair.bodyA;
        uint rowB = pair.bodyB;

        initColliderRow(rowA, manifoldIndex, a);
        initColliderRow(rowB, manifoldIndex, b);
        collisionPair.forceIndex = forceIndex;
        collisionPair.manifoldIndex = manifoldIndex;

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceTable->markAsDeleted(forceIndex + 0);
            forceTable->markAsDeleted(forceIndex + 1);
            forceIndex += 2;
            manifoldIndex++;
            continue;
        }

        // determine collision normal
        count++;
        ushort frontIndex = epa(a, b, collisionPair);
        vec2 normal = collisionPair.polytope[frontIndex].normal;

        manifoldNormals[manifoldIndex] = normal;

        // determine object overlap
        sat(a, b, collisionPair);

        // create manifold force in graph
        // TODO delay creation of these forces until after multithreading, these will insert into linked lists creating race conditions
        Manifold* aToB = new Manifold(this, bodyPointers[rowA], bodyPointers[rowB], forceIndex + 0); // A -> B
        Manifold* bToA = new Manifold(this, bodyPointers[rowB], bodyPointers[rowA], forceIndex + 1); // B -> A
        forcePointers[forceIndex + 0] = aToB;
        forcePointers[forceIndex + 1] = bToA;

        // set twins to allow for proper deletion
        aToB->getTwin() = bToA;
        bToA->getTwin() = aToB;

        // set special indices in each constructor, maybe set this ina constructor
        specialIndices[forceIndex + 0] = manifoldIndex;
        specialIndices[forceIndex + 1] = manifoldIndex;

        types[forceIndex + 0] = MANIFOLD;
        types[forceIndex + 1] = MANIFOLD;

        // increment enumeration
        forceIndex += 2;
        manifoldIndex++;
    }

    std::cout << "Positive Narrow:\t" << count << std::endl;
}

void Solver::reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex) {
    auto& bodyIndices = forceTable->getBodyIndex();

    forceTable->reserveManifolds(collisionPairs.size(), forceIndex, manifoldIndex);

    // assign forces their bodies
    uint i = 0;
    for (const auto& pair : collisionPairs) {
        bodyIndices[forceIndex + i + 0] = pair.bodyA;
        bodyIndices[forceIndex + i + 1] = pair.bodyB;
        i += 2;
    }
}

void Solver::initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow) {
    colliderRow.pos = bodyTable->getPos()[row];
    colliderRow.scale = bodyTable->getScale()[row];
    colliderRow.mat = bodyTable->getMat()[row];
    colliderRow.imat = bodyTable->getIMat()[row];
    colliderRow.start = meshFlat->getStartPtr(meshFlat->getStart()[bodyTable->getMesh()[row]]);

    // print("Mesh data");
    // print(bodyTable->getMesh()[row]);
    // print(meshTable->getStart()[bodyTable->getMesh()[row]]);

    colliderRow.length = meshFlat->getLength()[bodyTable->getMesh()[row]];
    colliderRow.simplex = getManifoldTable()->getSimplexPtr(manifoldIndex);
}

void Solver::warmstartManifolds() {
    // manifolds compute tangent and basis
    getManifoldTable()->warmstart();

    // compute rW
    auto& pos = bodyTable->getPos();
    auto& rmat = bodyTable->getRMat();
    auto& rAs = getManifoldTable()->getRA();
    auto& rBs = getManifoldTable()->getRB();
    auto& specials = forceTable->getSpecial();
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& isA = forceTable->getIsA();

    for (uint i = 0; i < forceTable->getSize(); i++) {
        uint specialIndex = specials[i];
        uint bodyIndex = bodyIndices[i];

        if (isA[i]) {
            rAs[specialIndex][0] = rmat[bodyIndex] * rAs[specialIndex][0];
            rAs[specialIndex][1] = rmat[bodyIndex] * rAs[specialIndex][1];
        } else {
            rBs[specialIndex][0] = rmat[bodyIndex] * rBs[specialIndex][0];
            rBs[specialIndex][1] = rmat[bodyIndex] * rBs[specialIndex][1];
        }
    }

    auto& bases = getManifoldTable()->getBasis();
    auto& tangents = getManifoldTable()->getTangent();
    auto& normals = getManifoldTable()->getNormal();
    auto& rAWs = getManifoldTable()->getRAW();
    auto& rBWs = getManifoldTable()->getRBW();
    auto& C0s = getManifoldTable()->getC0();
    auto& Js = forceTable->getJ();

    // set all C0 to zero
    for (uint i = 0; i < getManifoldTable()->getSize(); i++) {
        C0s[i] = Vec2Pair(); // TODO check if this defaults to zero
    }

    for (uint i = 0; i < forceTable->getSize(); i++) {
        uint specialIndex = specials[i];
        uint bodyIndex = bodyIndices[i];

        const mat2x2& basis = bases[specialIndex];
        const vec2& normal = normals[specialIndex];
        const vec2& tangent = tangents[specialIndex];

        // compute jacobians
        for (uint j = 0; j < 2; j++) {
            const vec2& rW = isA[i] ? rAWs[specialIndex][j] : rBWs[specialIndex][j];

            // Precompute the constraint and derivatives at C(x-), since we use a truncated Taylor series for contacts (Sec 4).
            // Note that we discard the second order term, since it is insignificant for contacts
            Js[i][2 * j + JN] = vec3{ basis[0][0], basis[0][1], cross(rW, normal) };
            Js[i][2 * j + JT] = vec3{ basis[1][0], basis[1][1], cross(rW, tangent) };

            C0s[specialIndex][j] += bases[specialIndex] * (xy(pos[bodyIndices[i]]) + rW) * (float) (2 * isA[i] - 1);
        }
    }
}

void Solver::warmstartForces() {
    forceTable->warmstart(alpha, gamma);
}

void Solver::warmstartBodies(float dt) {
    bodyTable->warmstartBodies(dt, gravity);
}

void Solver::updateVelocities(float dt) {
    bodyTable->updateVelocities(dt);
}

void Solver::mainloopPreload() {
    // set up all current dpX values since we do halfloads for compute constraints
    loadCdX(0, forceTable->getSize());
}

void Solver::primalUpdate(float dt) {
    auto& rhs = bodyTable->getRHS();
    auto& lhs = bodyTable->getLHS();
    auto& pos = bodyTable->getPos();
    auto& inertial = bodyTable->getInertial();
    auto& mass = bodyTable->getMass();
    auto& moment = bodyTable->getMoment();
    auto& bodyPtrs = bodyTable->getBodies();
    auto& lambdas = forceTable->getLambda();
    auto& stiffness = forceTable->getStiffness();
    auto& penalty = forceTable->getPenalty();
    auto& C = forceTable->getC();
    auto& motor = forceTable->getMotor();
    auto& fmax = forceTable->getFmax();
    auto& fmin = forceTable->getFmin();
    auto& H = forceTable->getH();
    auto& J = forceTable->getJ();

    for (uint b = 0; b < bodyTable->getSize(); b++) {
        // this is our falg for immovable objects
        if (mass[b] <= 0) {
            continue;
        }

        lhs[b] = glm::diagonal3x3(vec3{ mass[b], mass[b], moment[b] } / (dt * dt));
        rhs[b] = lhs[b] * (pos[b] - inertial[b]);

        // TODO replace this with known counting sort
        Rigid* body = bodyPtrs[b];
        for (Force* f = body->getForces(); f != nullptr; f = f->getNextA()) {
            uint forceIndex = f->getIndex();

            computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);
            computeDerivatives(forceIndex, forceIndex + 1, MANIFOLD);

            for (uint j = 0; j < MANIFOLD_ROWS; j++) {
                // Use lambda as 0 if it's not a hard constraint
                float lambda = glm::isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;

                // Compute the clamped force magnitude (Sec 3.2)
                float f = glm::clamp(penalty[forceIndex][j] * C[forceIndex][j] + lambda + motor[forceIndex][j], fmin[forceIndex][j], fmax[forceIndex][j]);

                // Compute the diagonally lumped geometric stiffness term (Sec 3.5)
                mat3x3 G = glm::diagonal3x3(vec3{ glm::length(H[forceIndex][j][0]), glm::length(H[forceIndex][j][1]), glm::length(H[forceIndex][j][2]) }) * abs(f);

                // Accumulate force (Eq. 13) and hessian (Eq. 17)
                rhs[b] += J[forceIndex][j] * f;
                lhs[b] += glm::outerProduct(J[forceIndex][j], J[forceIndex][j] * penalty[forceIndex][j]) + G;
            }
        }

        // Solve the SPD linear system using LDL and apply the update (Eq. 4)
        vec3 x;
        solve(lhs[b], x, rhs[b]);
        pos[b] -= x;
    }
}

void Solver::dualUpdate(float dt) {
    auto& lambdas = forceTable->getLambda();
    auto& stiffness = forceTable->getStiffness();
    auto& penalty = forceTable->getPenalty();
    auto& fmax = forceTable->getFmax();
    auto& fmin = forceTable->getFmin();
    auto& C = forceTable->getC();

    for (uint forceIndex = 0; forceIndex < forceTable->getSize(); forceIndex++) {
        computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);

        for (uint j = 0; j < MANIFOLD_ROWS; j++) {
            // Use lambda as 0 if it's not a hard constraint
            float lambda = glm::isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;

            // Update lambda (Eq 11)
            // Note that we don't include non-conservative forces (ie motors) in the lambda update, as they are not part of the dual problem.
            float f = glm::clamp(penalty[forceIndex][j] * C[forceIndex][j] + lambda, fmin[forceIndex][j], fmax[forceIndex][j]);

            // TODO add fracture
    
            // Update the penalty parameter and clamp to material stiffness if we are within the force bounds (Eq. 16)
            if (lambdas[forceIndex][j] > fmin[forceIndex][j] && lambdas[forceIndex][j] < fmax[forceIndex][j]) {
                penalty[forceIndex][j] = glm::min(penalty[forceIndex][j] + beta * abs(C[forceIndex][j]), PENALTY_MAX, stiffness[forceIndex][j]);
            }
        }
    }
}

void Solver::computeConstraints(uint start, uint end, ushort type) {
    auto& pos = bodyTable->getPos();
    auto& initial = bodyTable->getInitial();
    auto& J = forceTable->getJ();
    auto& friction = getManifoldTable()->getFriction();
    auto& fmax = forceTable->getFmax();
    auto& fmin = forceTable->getFmin();
    auto& C = forceTable->getC();
    auto& lambda = forceTable->getLambda();
    auto& C0 = getManifoldTable()->getC0();
    auto& cdA = getManifoldTable()->getCdA();
    auto& cdB = getManifoldTable()->getCdB();

    // other dp will already be loaded, only focus on self
    loadCdX(start, end);

    for (uint i = start; i < end; i++) {
        for (uint j = 0; j < 2; j++) {
            // Compute the Taylor series approximation of the constraint function C(x) (Sec 4)
            C[i][2 * j + JN] = C0[i][j].x * (1 - alpha) + cdA[i][2 * j + JN] + cdB[i][2 * j + JN];
            C[i][2 * j + JT] = C0[i][j].y * (1 - alpha) + cdA[i][2 * j + JT] + cdB[i][2 * j + JT];

            float frictionBound = abs(lambda[i][2 * j] * friction[i]);
            fmax[i][2 * j + JT] = frictionBound;
            fmin[i][2 * j + JT] = frictionBound;

            // TODO add force stick
        }
    }
}

void Solver::computeDerivatives(uint start, uint end, ushort type) {
    // Manifolds do not need to compute derivatives
    if (type == 0) {
        return;
    }

    for (uint i = start; i < end; i++) {

    }
}

void Solver::loadCdX(uint start, uint end) {
    auto& pos = bodyTable->getPos();
    auto& initial = bodyTable->getInitial();
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& specialIndices = forceTable->getSpecial();
    auto& isA = forceTable->getIsA();
    auto& J = forceTable->getJ();
    auto& cdA = getManifoldTable()->getCdA();
    auto& cdB = getManifoldTable()->getCdB();

    for (uint i = start; i < end; i++) {
        uint special = specialIndices[i];
        uint body = bodyIndices[i];

        FloatROWS& cdX = isA[i] ? cdA[special] : cdB[special];

        for (ushort j = 0; j < 2; j++) {
            cdX[2 * j + JN] = glm::dot(J[i][2 * j + JN], pos[body] - initial[body]);
            cdX[2 * j + JT] = glm::dot(J[i][2 * j + JT], pos[body] - initial[body]);
        }
    }
}

void Solver::draw() {
    // TODO draw everything
}