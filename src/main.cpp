#include "util/includes.h"
#include "game/game.h"
#include "levels/levels.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    // image
    game->addImage("man", new Image("textures/man.png"));
    game->addImage("paper", new Image("textures/paper.png"));
    game->addImage("box", new Image("textures/container.jpg"));

    // material
    game->addMaterial("man", new Material({ 1, 1, 1 }, game->getImage("man")));
    game->addMaterial("paper", new Material({ 1, 1, 1 }, game->getImage("paper")));
    game->addMaterial("box", new Material({ 1, 1, 1 }, game->getImage("box")));

    // mesh
    game->addMesh("quad", new Mesh("models/quad.obj"));
    game->addMesh("paper", new Mesh("models/paper.obj"));

    // collider
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));
    game->addCollider("ugh", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-1.f, 1.f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));

    // create a piece of paper for a demo
    // std::vector<std::vector<vec2>> paperDemoPolygon = {
    //     {{10.0, 10.0}, {40.0, 10.0}, {40.0, 30.0}, {10.0, 30.0}},
    //     {{20.0, 20.0}, {30.0, 20.0}, {30.0, 25.0}, {20.0, 25.0}},
    //     {{20.0, 12.5}, {30.0, 12.5}, {27.5, 15.0}}
    // };
    // for (uint i = 0; i < paperDemoPolygon.size(); i++) {
    //     for (uint j = 0; j < paperDemoPolygon.at(i).size(); j++) {
    //         paperDemoPolygon[i][j] *= 0.3;
    //         paperDemoPolygon[i][j] += vec2{-7.5, -6.5};
    //     }   
    // }
    // std::vector<uint> paperDemoIndices;
    // std::vector<float> paperDemoData;
    // Navmesh::earcut(paperDemoPolygon, paperDemoIndices);
    // Navmesh::convertToMesh(paperDemoPolygon, paperDemoIndices, paperDemoData);
    // game->addMesh("paper", new Mesh(paperDemoData));

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .scale={1, 1}, .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    Node2D* enemyNode = new Node2D(game->getScene(), { .position={3, 4}, .scale={0.7, 0.7}, .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .collider=game->getCollider("quad") });
    game->addEnemy(new Enemy(3, 2, enemyNode, nullptr, nullptr));

    // polygon clip test
    Clipper2Lib::PathsD subject = {{{0,0}, {100,0}, {100,100}, {0,100}}};
    Clipper2Lib::PathsD clip = {{{50,-50}, {150,-50}, {150,50}, {50,50}}};

    Clipper2Lib::PathsD result = Clipper2Lib::Intersect(subject, clip, Clipper2Lib::FillRule::NonZero);

    std::cout << "Intersection result count: " << result.size() << "\n";

    // Loop through each path (polygon)
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "Polygon " << i << ":\n";
        for (const Clipper2Lib::PointD& p : result[i]) {
            std::cout << "  (" << p.x << ", " << p.y << ")\n";
        }
    }

    // create test paper
    game->setPaper(new Paper(
        game->getMesh("paper"), 
        {{2.0, 1.5}, {-2.0, 1.5}, {-2.0, -1.5}, {2.0, -1.5}}
    ));

    // background paper
    // Node2D* paper = new Node2D(game->getScene(), { .mesh=game->getMesh("paper"), .material=game->getMaterial("paper") });

    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 120);
    }

    delete game;
}

// #include "util/includes.h"
// #include "levels/triangle.h"
// #include "levels/dymesh.h"
// #include <iostream>

// int main() {
//     // --- Define simple rectangular DyMesh A ---
//     std::vector<vec2> regionA = {
//         {0.0f, 0.0f},
//         {3.0f, 0.0f},
//         {3.0f, 3.0f},
//         {0.0f, 3.0f}
//     };

//     // Triangulate A manually for now
//     std::vector<Tri> dataA = {
//         Tri({Vert({0,0},{0,0}), Vert({3,0},{1,0}), Vert({3,3},{1,1})}),
//         Tri({Vert({0,0},{0,0}), Vert({3,3},{1,1}), Vert({0,3},{0,1})})
//     };

//     DyMesh meshA(regionA, dataA);

//     // --- Define smaller overlapping DyMesh B ---
//     std::vector<vec2> regionB = {
//         {2.0f, 2.0f},
//         {3.0f, 2.0f},
//         {3.0f, 3.0f},
//         {2.0f, 3.0f}
//     };

//     std::vector<Tri> dataB = {
//         Tri({Vert({2,2},{0,0}), Vert({3,2},{1,0}), Vert({3,3},{1,1})}),
//         Tri({Vert({2,2},{0,0}), Vert({3,3},{1,1}), Vert({2,3},{0,1})})
//     };

//     DyMesh meshB(regionB, dataB);

//     // --- Initial UV sample tests ---
//     vec2 sampleA = meshA.sampleUV({1.0f, 1.0f});
//     vec2 sampleB = meshB.sampleUV({3.0f, 3.0f});
//     std::cout << "Sample A UV: " << sampleA.x << ", " << sampleA.y << std::endl;
//     std::cout << "Sample B UV: " << sampleB.x << ", " << sampleB.y << std::endl;

//     // --- Perform copy: copy UVs from B into A (if contained) ---
//     meshB.copy(meshA);
//     std::cout << "Copy completed. UVs in overlapping area should now match meshA." << std::endl;

//     // --- Perform cut: remove B region from A ---
//     meshA.cut(meshB.region);
//     std::cout << "Cut completed. Region A now has " << meshA.region.size() << " vertices." << std::endl;

//     // --- Perform paste: merge B into A ---
//     meshA.paste(meshB);
//     std::cout << "Paste completed. Region A now has " << meshA.region.size() << " vertices." << std::endl;

//     // --- Sample after paste ---
//     vec2 sampleMerged = meshA.sampleUV({3.0f, 3.0f});
//     std::cout << "Sample merged UV: " << sampleMerged.x << ", " << sampleMerged.y << std::endl;

//     std::vector<float> data;
//     meshA.toData(data);

//     // render
//     Engine* engine = new Engine(800, 800, "");
//     Scene2D* scene = new Scene2D(engine);
//     Image* image = new Image("textures/paper.png");
//     Material* material = new Material({0, 0, 0}, image);
//     Mesh* mesh = new Mesh(data);

//     new Node2D(scene, { .mesh=mesh, .material=material });

//     while (engine->isRunning()) {
//         engine->update();
//         scene->update(0.00001);
//         scene->render();
//         engine->render();
//     }

//     delete mesh;
//     delete material;
//     delete scene;
//     delete engine;

//     std::cout << "Smoke test complete." << std::endl;
//     return 0;
// }
