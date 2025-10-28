#include <basilisk/basilisk.h>

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene* scene = new Scene(engine);
    Scene2D* scene2D = new Scene2D(engine);

    // Data for making node
    Mesh* cube = new Mesh("models/cube.obj");
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    Node* parent = new Node(scene, scene->getShader(), cube, texture);
    Node* node = new Node(parent, scene->getShader(), cube, texture);
    new Node(node, scene->getShader(), cube, texture);
    Node* clone = node;
    clone = std::move(parent);

    Node2D* square = new Node2D(scene2D, scene2D->getShader(), quad, texture);

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene->update();
        scene->render();

        scene2D->update();
        scene2D->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete cube;
    delete node;
    delete quad;
    delete scene;
    delete scene2D;
    delete engine;
}