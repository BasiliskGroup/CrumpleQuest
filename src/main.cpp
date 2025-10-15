#include "solver/physics.h"
#include "util/random.h"
#include "util/time.h"
#include "crumpleQuest/character/character.h"
#include "scene/basilisk.h"

// int main() {
//     Solver* solver = new Solver();
//     Mesh* cubeMesh = new Mesh(solver, {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}});

//     float dx = 5;
//     float dr = 2 * 3.14;

//     // create a list of rigids
//     std::vector<Rigid*> objects;
//     for (int i = 0; i < 100; i++) {
//         objects.push_back(new Rigid(solver, {uniform(-dx, dx), uniform(-dx, dx), uniform(0, dr)}, {1, 1}, 1, 0.4, {0, 0, 0}, cubeMesh));
//     }

//     for (int i = 0; i < 10; i++) {
//         solver->step(1.0 / 60.0);

//         // testing mid simulation body deletion
//         int deleteIndex = randrange(0, objects.size());
//         delete objects[deleteIndex];
//         objects.erase(objects.begin() + deleteIndex);
//     }

//     // delete mesh
//     delete cubeMesh;

//     // deleting solver should always be last
//     delete solver;
//     return 0;
// }

int main() {
    Scene* scene = new Scene();

    // expose nodes
    std::vector<Node*> nodes;

    for (uint i = 0; i < 2000; i++) {
        Node* node = new Node(scene);
        
        // add to random spot in tree
        int push = randint(0, nodes.size());
        if (push == nodes.size()) scene->getRoot()->add(node);
        else nodes[push]->add(node);

        nodes.push_back(node);
    }

    // 1 so we dont delete root
    int deleteIndex = randrange(0, nodes.size());
    delete nodes[deleteIndex];
    nodes.erase(nodes.begin() + deleteIndex);

    delete scene;
}