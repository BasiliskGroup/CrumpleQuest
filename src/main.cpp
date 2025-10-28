#include <basilisk/basilisk.h>

int main() {
    Engine* engine = new Engine(800, 800, "Basilisk");
    Scene2D* scene2D = new Scene2D(engine);

    // Data for making node
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
        
    Node2D* square = new Node2D(scene2D, { .mesh=quad, .texture=texture });

    // Main loop continues as long as the window is open
    while (engine->isRunning()) {
        engine->update();

        scene2D->update();
        scene2D->render();

        engine->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete quad;
    delete scene2D;
    delete engine;
}