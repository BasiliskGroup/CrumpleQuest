#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"

// ----- Debug Main -----
int main() {
    Solver* solver = new Solver();
    Mesh* cubeMesh = new Mesh(solver, {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}});

    float dx = 10;
    float dr = 2 * 3.14;

    // create a list of rigids
    std::vector<Rigid*> objects;
    for (int i = 0; i < 100; i++) {
        objects.push_back(new Rigid(solver, {uniform(-dx, dx), uniform(-dx, dx), uniform(0, dr)}, {1, 1}, 1, 0.4, {0, 0, 0}, cubeMesh));
    }

    int deleteIndex = randrange(0, objects.size());
    delete objects[deleteIndex];
    objects.erase(objects.begin() + deleteIndex);

    solver->getMeshSoA()->compact();

    for (int i = 0; i < 10; i++) {
        solver->step(1.0 / 60.0);
    }

    print("finished stepping");

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

// #include "physics.h"
// #include "util/random.h"
// #include "util/time.h"
// #include <cmath>

// int main() {
//     Solver* solver = new Solver();
    
//     // Define square mesh
//     std::vector<glm::vec2> square_points = {
//         {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}
//     };
//     Mesh* square_mesh = new Mesh(solver, square_points);
    
//     // Test Case 1: Direct head-on collision (will collide)
//     new Rigid(solver, {-5.0f, 5.0f, 0.0f}, {3.5f, 3.5f}, 1.0f, 0.4f, {1.0f, 0.0f, 0.0f}, square_mesh);
//     new Rigid(solver, {-3.0f, 5.0f, 0.0f}, {1.0f, 1.0f}, 1.0f, 0.4f, {0.0f, 1.0f, 0.0f}, square_mesh);
    
//     // Test Case 2: Corner-to-corner (barely touching, will collide)
//     new Rigid(solver, {-5.0f, 2.0f, 0.0f}, {1.0f, 1.0f}, 1.0f, 0.4f, {1.0f, 0.5f, 0.0f}, square_mesh);
//     new Rigid(solver, {-3.6f, 3.4f, 0.0f}, {1.0f, 1.0f}, 1.0f, 0.4f, {0.0f, 0.5f, 1.0f}, square_mesh);
    
//     // Test Case 3: Rotated squares close together (pass broad, might fail GJK if not overlapping)
//     new Rigid(solver, {0.0f, 5.0f, static_cast<float>(M_PI/4)}, {1.2f, 0.7f}, 1.0f, 0.4f, {1.0f, 0.0f, 1.0f}, square_mesh);
//     new Rigid(solver, {1.5f, 5.4f, static_cast<float>(-M_PI/4)}, {1.2f, 1.2f}, 1.0f, 0.4f, {1.0f, 1.0f, 0.0f}, square_mesh);
    
//     // Test Case 4: Thin rectangles at angles (bounding circles overlap, shapes might not)
//     new Rigid(solver, {-7.0f, -2.0f, static_cast<float>(M_PI/6)}, {1.0f, 1.0f}, 1.0f, 0.4f, {0.8f, 0.2f, 0.2f}, square_mesh);
//     new Rigid(solver, {-6.5f, -2.0f, static_cast<float>(-M_PI/6)}, {1.0f, 0.6f}, 1.0f, 0.4f, {0.2f, 0.8f, 0.2f}, square_mesh);
    
//     // Add static boundary for containment
//     new Rigid(solver, {0.0f, -10.0f, 0.0f}, {30.0f, 1.0f}, -1.0f, 0.4f, {0.4f, 0.4f, 0.4f}, square_mesh);
    
//     // Step physics once (equivalent to running = False in Python)
//     solver->step(0.016f);
    
//     // Cleanup
//     delete square_mesh;
//     delete solver;
    
//     return 0;
// }