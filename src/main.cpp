#include <basilisk/basilisk.h>

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene2D = new Scene2D(engine);

    // Data for making node
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/man.png");
    Material* material1 = new Material({1.0, 1.0, 0.0}, image);
        
    Node2D* square = new Node2D(scene2D, { .mesh=quad, .material=material1 });

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene2D->update(1 / 60);
        scene2D->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete material1;
    delete quad;
    delete scene2D;
    delete engine;
}