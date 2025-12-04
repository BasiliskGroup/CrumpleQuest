#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include "character/behavior.h"
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
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey", "test", "knight", "table", "sword", "gun", "bullet", "wand", "green", "red", "black", "empty", "yellow" };
    for (std::string& name : imageNames) {
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }

    // Player
    game->addAnimation("player_idle", "art/sprites/player/idle/", 4);
    game->addAnimation("player_run", "art/sprites/player/run/", 3);
    game->addAnimation("player_attack", "art/sprites/player/attack/", 4);
    // Pencil
    game->addAnimation("pencil_idle", "art/sprites/player/weapons/pencil/idle/", 4);
    game->addAnimation("pencil_run", "art/sprites/player/weapons/pencil/run/", 3);
    game->addAnimation("pencil_attack", "art/sprites/player/weapons/pencil/attack/", 4);
    // Clipfly
    game->addAnimation("clipfly_idle", "art/sprites/enemies/contact/notebook_clipfly/idle/", 4);
    game->addAnimation("clipfly_attack", "art/sprites/enemies/contact/notebook_clipfly/attack/", 4);
    // Staple
    game->addAnimation("staple_idle", "art/sprites/enemies/melee/notebook_staple/idle/", 4);
    game->addAnimation("staple_attack", "art/sprites/enemies/melee/notebook_staple/attack/", 6);
    // Glue
    game->addAnimation("glue_idle", "art/sprites/enemies/ranged/notebook_glue/idle/", 6);
    game->addAnimation("glue_attack", "art/sprites/enemies/ranged/notebook_glue/attack/", 7);
    // Integral
    game->addAnimation("integral_idle", "art/sprites/enemies/contact/grid_integral/idle/", 4);
    game->addAnimation("integral_attack", "art/sprites/enemies/contact/grid_integral/attack/", 9);
    // Sigma
    game->addAnimation("sigma_idle", "art/sprites/enemies/melee/grid_sigma/idle/", 5);
    game->addAnimation("sigma_attack", "art/sprites/enemies/melee/grid_sigma/attack/", 6);
    // Pi
    game->addAnimation("pi_idle", "art/sprites/enemies/ranged/grid_pi/idle/", 6);
    game->addAnimation("pi_attack", "art/sprites/enemies/ranged/grid_pi/attack/", 7);

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper0", "paper1" };
    for (std::string& name : meshNames) game->addMesh(name, new Mesh("models/" + name + ".obj"));

    // ------------------------------------------
    // Load game start
    // ------------------------------------------

    // Setup background music
    auto parchment_track = game->getAudio().CreateTrack();
    game->getAudio().AddLayer(parchment_track, "parchment", "sounds/parchment.wav", "music");
    game->getAudio().SetLayerVolume(parchment_track, "parchment", 1.0f);
    game->getAudio().PlayTrack(parchment_track);

    // Initialize behavior system
    BehaviorRegistry::initialize();
    
    // load levels
    Enemy::generateTemplates(game);
    SingleSide::generateTemplates(game);
    PaperMesh::generateTemplates(game);
    Paper::generateTemplates(game);

    // initialize menus (menu scene is ready)
    game->initMenus();

    while (game->getEngine()->isRunning()) {
        game->update(game->getEngine()->getDeltaTime());
    }
    
    delete game;
}