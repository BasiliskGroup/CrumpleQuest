#include "solver/physics.h"


Solver::Solver() : forces(nullptr), bodies(nullptr) {
    // set default params
    gravity = -10.0f;
    iterations = 10;
    beta = 100000.0f;
    alpha = 0.99f;
    gamma = 0.99f;

    // create SoAs
    forceSoA = new ForceSoA(1024);
    bodySoA = new BodySoA(512);
    meshSoA = new MeshSoA(256, 16);
}

Solver::~Solver() {
    // delete bodies
    while (bodies) {
        delete bodies;
    }

    delete bodySoA;

    // delete forces
    while (forces) {
        delete forces;
    }

    delete forceSoA;
    delete meshSoA;
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
    bodySoA->computeTransforms();
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
    bodySoA->compact();

    // route forces back to their correct bodies after standard compact
    auto& bodyIndices = forceSoA->getBodyIndex();
    auto& toDelete = forceSoA->getToDelete();
    auto& inverseForceMap = bodySoA->getInverseForceMap();

    for (uint i = 0; i < forceSoA->getSize(); i++) {
        // will index out of bounds
        if (toDelete[i] == true) {
            continue;
        }
        bodyIndices[i] = inverseForceMap[bodyIndices[i]];
    }
}

void Solver::compactForces() {
    forceSoA->compact();
}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    auto& pos = bodySoA->getPos();
    auto& radii = bodySoA->getRadius();

    // clear contact pairs from last frame
    collisionPairs.clear();

    vec2 dpos;
    float dy;
    float radsum;
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext()) {
        for (Rigid* bodyB = bodyA->getNext(); bodyB != nullptr; bodyB = bodyB->getNext()) {
            // ignore collision flag
            if (bodyA->constrainedTo(bodyB)) {
                continue;
            }

            uint i = bodyA->getIndex();
            uint j = bodyB->getIndex();

            dpos = pos[i] - pos[j];
            radsum = radii[i] + radii[j];
            if (radsum * radsum > glm::length2(dpos)) {
                collisionPairs.emplace_back(i, j);
            }
        }
    }
}

void Solver::narrowCollision() {
    auto& manifoldNormals = getManifoldSoA()->getNormal();
    auto& forcePointers = forceSoA->getForces();
    auto& bodyPointers = bodySoA->getBodies();
    auto& specialIndices = forceSoA->getSpecial();

    // reserve space to perform all collisions 
    uint forceIndex, manifoldIndex;
    reserveForcesForCollision(forceIndex, manifoldIndex);

    int count = 0; // TODO debug variable

    ColliderRow a, b;
    CollisionPair collisionPair;
    for (const auto& pair : collisionPairs) {
        uint rowA = pair.first;
        uint rowB = pair.second;

        initColliderRow(rowA, manifoldIndex, a);
        initColliderRow(rowB, manifoldIndex, b);
        collisionPair.forceIndex = forceIndex;
        collisionPair.manifoldIndex = manifoldIndex;

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceSoA->markForDeletion(forceIndex + 0);
            forceSoA->markForDeletion(forceIndex + 1);
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
        forcePointers[forceIndex + 0] = new Manifold(this, (Rigid*) bodyPointers[rowA], (Rigid*) bodyPointers[rowB], forceIndex + 0); // A -> B
        forcePointers[forceIndex + 1] = new Manifold(this, (Rigid*) bodyPointers[rowB], (Rigid*) bodyPointers[rowA], forceIndex + 1); // B -> A

        // set special indices in each constructor, maybe set this ina constructor
        specialIndices[forceIndex + 0] = manifoldIndex;
        specialIndices[forceIndex + 1] = manifoldIndex;

        // increment enumeration
        forceIndex += 2;
        manifoldIndex++;
    }

    std::cout << "Positive Narrow:\t" << count << std::endl;
}

void Solver::reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex) {
    auto& bodyIndices = forceSoA->getBodyIndex();

    forceSoA->reserveManifolds(collisionPairs.size(), forceIndex, manifoldIndex);

    // assign forces their bodies
    uint i = 0;
    for (const auto& pair : collisionPairs) {
        bodyIndices[forceIndex + i + 0] = pair.first;
        bodyIndices[forceIndex + i + 1] = pair.second;
        i += 2;
    }
}

void Solver::initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow) {
    colliderRow.pos = bodySoA->getPos()[row];
    colliderRow.scale = bodySoA->getScale()[row];
    colliderRow.mat = bodySoA->getMat()[row];
    colliderRow.imat = bodySoA->getIMat()[row];
    colliderRow.start = meshSoA->getStartPtr(meshSoA->getStart()[bodySoA->getMesh()[row]]);

    // print("Mesh data");
    // print(bodySoA->getMesh()[row]);
    // print(meshSoA->getStart()[bodySoA->getMesh()[row]]);

    colliderRow.length = meshSoA->getLength()[bodySoA->getMesh()[row]];
    colliderRow.simplex = getManifoldSoA()->getSimplexPtr(manifoldIndex);
}

void Solver::warmstartManifolds() {
    // manifolds compute tangent and basis
    getManifoldSoA()->warmstart();

    // compute rW
    auto& pos = bodySoA->getPos();
    auto& rmat = bodySoA->getRMat();
    auto& rAs = getManifoldSoA()->getRA();
    auto& rBs = getManifoldSoA()->getRB();
    auto& specials = forceSoA->getSpecial();
    auto& bodyIndices = forceSoA->getBodyIndex();
    auto& isA = forceSoA->getIsA();

    for (uint i = 0; i < forceSoA->getSize(); i++) {
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

    auto& bases = getManifoldSoA()->getBasis();
    auto& tangents = getManifoldSoA()->getTangent();
    auto& normals = getManifoldSoA()->getNormal();
    auto& rAWs = getManifoldSoA()->getRAW();
    auto& rBWs = getManifoldSoA()->getRBW();
    auto& C0s = getManifoldSoA()->getC0();
    auto& Js = forceSoA->getJ();

    // set all C0 to zero
    for (uint i = 0; i < getManifoldSoA()->getSize(); i++) {
        C0s[i] = Vec2Pair(); // TODO check if this defaults to zero
    }

    for (uint i = 0; i < forceSoA->getSize(); i++) {
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
    forceSoA->warmstart(alpha, gamma);
}

void Solver::warmstartBodies(float dt) {
    bodySoA->warmstartBodies(dt, gravity);
}

void Solver::updateVelocities(float dt) {
    bodySoA->updateVelocities(dt);
}

void Solver::mainloopPreload() {
    // set up all current dpX values since we do halfloads for compute constraints
    loadCdX(0, forceSoA->getSize());
}

void Solver::primalUpdate(float dt) {
    auto& rhs = bodySoA->getRHS();
    auto& lhs = bodySoA->getLHS();
    auto& pos = bodySoA->getPos();
    auto& inertial = bodySoA->getInertial();
    auto& mass = bodySoA->getMass();
    auto& moment = bodySoA->getMoment();
    auto& bodyPtrs = bodySoA->getBodies();
    auto& lambdas = forceSoA->getLambda();
    auto& stiffness = forceSoA->getStiffness();
    auto& penalty = forceSoA->getPenalty();
    auto& C = forceSoA->getC();
    auto& motor = forceSoA->getMotor();
    auto& fmax = forceSoA->getFmax();
    auto& fmin = forceSoA->getFmin();
    auto& H = forceSoA->getH();
    auto& J = forceSoA->getJ();

    for (uint b = 0; b < bodySoA->getSize(); b++) {
        // this is our falg for immovable objects
        if (mass[b] <= 0) {
            continue;
        }

        lhs[b] = glm::diagonal3x3(vec3{ mass[b], mass[b], moment[b] } / (dt * dt));
        rhs[b] = lhs[b] * (pos[b] - inertial[b]);

        // TODO replace this with known counting sort
        Rigid* body = (Rigid*) bodyPtrs[b];
        for (Force* f = body->getForces(); f != nullptr; f = f->getNextA()) {
            uint forceIndex = f->getIndex();

            computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);
            computeDerivatives(forceIndex, forceIndex + 1, MANIFOLD);

            for (uint j = 0; j < MANIFOLD_ROWS; j++) {
                // Use lambda as 0 if it's not a hard constraint
                float lambda = isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;

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
    auto& lambdas = forceSoA->getLambda();
    auto& stiffness = forceSoA->getStiffness();
    auto& penalty = forceSoA->getPenalty();
    auto& fmax = forceSoA->getFmax();
    auto& fmin = forceSoA->getFmin();
    auto& C = forceSoA->getC();

    for (uint forceIndex = 0; forceIndex < forceSoA->getSize(); forceIndex++) {
        computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);

        for (uint j = 0; j < MANIFOLD_ROWS; j++) {
            // Use lambda as 0 if it's not a hard constraint
            float lambda = isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;

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
    auto& pos = bodySoA->getPos();
    auto& initial = bodySoA->getInitial();
    auto& J = forceSoA->getJ();
    auto& friction = getManifoldSoA()->getFriction();
    auto& fmax = forceSoA->getFmax();
    auto& fmin = forceSoA->getFmin();
    auto& C = forceSoA->getC();
    auto& lambda = forceSoA->getLambda();
    auto& C0 = getManifoldSoA()->getC0();
    auto& cdA = getManifoldSoA()->getCdA();
    auto& cdB = getManifoldSoA()->getCdB();

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
    auto& pos = bodySoA->getPos();
    auto& initial = bodySoA->getInitial();
    auto& bodyIndices = forceSoA->getBodyIndex();
    auto& specialIndices = forceSoA->getSpecial();
    auto& isA = forceSoA->getIsA();
    auto& J = forceSoA->getJ();
    auto& cdA = getManifoldSoA()->getCdA();
    auto& cdB = getManifoldSoA()->getCdB();

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