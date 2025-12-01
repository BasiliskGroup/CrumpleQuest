#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include "audio/audio_manager.h"
#include "ui/menu_manager.h"
#include "resource/animation.h"
#include "resource/animator.h"
#include "weapon/weapon.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    // ------------------------------------------
    // Load resources
    // ------------------------------------------

    // image and material
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey", "test", "knight", "table", "sword", "gun", "bullet", "wand" };
    for (std::string& name : imageNames) {
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper0", "paper1" };
    for (std::string& name : meshNames) game->addMesh(name, new Mesh("models/" + name + ".obj"));

    // ------------------------------------------
    // Load game start
    // ------------------------------------------

    // audio system setup
    if (!game->getAudio().Initialize()) {
        std::cerr << "Failed to initialize audio system\n";
        // return 1;
    }
    std::cout << "Audio system initialized successfully\n";
    
    game->getAudio().SetMasterVolume(1.0f);
    
    auto music_group = game->getAudio().CreateGroup("music");
    
    game->getAudio().SetGroupVolume(music_group, 0.7f);
    
    auto parchment_track = game->getAudio().CreateTrack();
    game->getAudio().AddLayer(parchment_track, "parchment", "sounds/parchment.wav", "music");
    game->getAudio().SetLayerVolume(parchment_track, "parchment", 1.0f);
    
    game->getAudio().PlayTrack(parchment_track);

    // load levels
    SingleSide::generateTemplates(game);
    Paper::generateTemplates(game);
    game->setPaper("empty");

    game->getPaper()->regenerateWalls();

    // initialize menus (after scene is ready)
    game->initMenus();
    
    game->getMenus()->showMainMenu();

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={1, 1}, .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, game->getSide(), nullptr);
    game->setPlayer(player);

    // create weapons
    player->setWeapon(new MeleeWeapon(player, { .mesh=game->getMesh("quad"), .material=game->getMaterial("sword"), .scale={0.75, 0.75}}, { .damage=1, .life=0.2f, .radius=0.5 }, 30.0f));

    // ------------------------------------------
    // Testing
    // ------------------------------------------

    Animation* animation = new Animation({game->getMaterial("box"), game->getMaterial("man"), game->getMaterial("knight")});
    Animator* playerAnimator = new Animator(game->getEngine(), playerNode, animation);
    playerAnimator->setFrameRate(1);

    // test add button
    Button* testButton = new Button(game, { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .position={-2, -2}, .scale={0.5, 0.5} }, { 
        .onDown=[]() { std::cout << "Button Pressed" << std::endl; }
    });

    // add temp background paper
    Node2D* paperBackground = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("paper"), .scale={16, 9} });
    paperBackground->setLayer(-0.9);

    // spawn enemy on click
    testButton->setOnUp([game]() {
        Node2D* enemyNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position={3, 4}, .scale={0.7, 0.7}, .collider=game->getCollider("quad") });
        game->addEnemy(new Enemy(3, 0.1, enemyNode, game->getSide(), nullptr, nullptr));
    });

    game->addUI(testButton);

    // slider
    Slider* testSlider = new Slider(game, { -4, 4 }, { 0, 4 }, { .pegMaterial=game->getMaterial("box") });
    testSlider->setCallback([game](float proportion) {
        for (Enemy* enemy : game->getEnemies()) {
            enemy->setSpeed(5 * proportion);
        }
    });
    game->addUI(testSlider);

    while (game->getEngine()->isRunning()) {
        game->update(game->getEngine()->getDeltaTime());
        playerAnimator->update();
    }
    
    delete game;
}