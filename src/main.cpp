#include "physics.h"
#include "util/random.h"
#include <thread>  // Required for std::this_thread::sleep_for
#include <chrono>  // Required for std::chrono::seconds

// ----- Debug Main -----
int main() {
    Solver* solver = new Solver();
    Mesh* cubeMesh = new Mesh(solver, {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}});

    float dx = 1;

    // create a list of rigids
    std::vector<Rigid*> objects;
    for (int i = 0; i < 10; i++) {
        objects.push_back(new Rigid(solver, {uniform(-dx, dx), uniform(-dx, dx), uniform(-dx, dx)}, {1, 1}, 1, 0.4, {0, 0, 0}, cubeMesh));
    }

    int deleteIndex = randrange(0, objects.size());
    delete objects[deleteIndex];
    objects.erase(objects.begin() + deleteIndex);

    solver->getMeshSoA()->compact();

    for (int i = 0; i < 60; i++) {
        solver->step(1.0 / 60.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    solver->getMeshSoA()->compact();

    deleteIndex = randrange(0, objects.size());
    delete objects[deleteIndex];
    objects.erase(objects.begin() + deleteIndex);

    // delete mesh
    delete cubeMesh;

    // deleting solver should always be last
    delete solver;
    return 0;
}
