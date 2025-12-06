#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include "character/behavior.h"
#include "character/attackAction.h"
#include "character/moveAction.h"
#include "audio/audio_manager.h"
#include "audio/music_player.h"
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
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey", "test", "knight", "table", "sword", "gun", "bullet", "wand", "green", "red", "black", "empty", "yellow", "rug_desaturated", "john", "blue", "darkred", "circle" };
    for (std::string& name : imageNames) {
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }
    
    // Load notebook background from art/assets
    game->addImage("notebook", new Image("art/assets/notebook.PNG"));
    game->addMaterial("notebook", new Material({ 1, 1, 1 }, game->getImage("notebook")));

    // Load Hands
    game->addImage("hand_up", new Image("art/assets/hands/up.PNG"));
    game->addMaterial("hand_up", new Material({ 1, 1, 1 }, game->getImage("hand_up")));
    game->addImage("hand_down", new Image("art/assets/hands/down.PNG"));
    game->addMaterial("hand_down", new Material({ 1, 1, 1 }, game->getImage("hand_down")));
    game->addImage("hand_left", new Image("art/assets/hands/left.PNG"));
    game->addMaterial("hand_left", new Material({ 1, 1, 1 }, game->getImage("hand_left")));
    game->addImage("hand_right", new Image("art/assets/hands/right.PNG"));
    game->addMaterial("hand_right", new Material({ 1, 1, 1 }, game->getImage("hand_right")));
    
    std::unordered_map<std::string, std::vector<std::string>> levelNames = {
        { "notebook", { "blank", "level1", "level2", "level3", "level4", "level5"} },
        {"tutorial", { "tutorial" } }
    };
    for (auto& [name, levels] : levelNames) {
        for (std::string& level : levels) {
            game->addImage(name + "_" + level, new Image("art/maps/" + name + "/" + level + ".PNG"));
            game->addMaterial(name + "_" + level, new Material({ 1, 1, 1 }, game->getImage(name + "_" + level)));
        }
    }

    for (int i = 1; i < 6; i++) {
        game->addImage("piProjectile" + std::to_string(i), new Image("art/sprites/enemies/ranged/grid_pi/projectiles_pi/" + std::to_string(i) + ".PNG"));
        game->addMaterial("piProjectile" + std::to_string(i), new Material({ 1, 1, 1 }, game->getImage("piProjectile" + std::to_string(i))));
    }

    game->addImage("glueProjectile", new Image("art/sprites/enemies/ranged/notebook_glue/projectile_glue.PNG"));
    game->addMaterial("glueProjectile", new Material({ 1, 1, 1 }, game->getImage("glueProjectile")));

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
    // Heart
    game->addAnimation("heart", "art/assets/heart/", 7);

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper0", "paper1", "quad3D", "cube", "mug", "john", "heart", "sphere"};
    for (std::string& name : meshNames) game->addMesh(name, new Mesh("models/" + name + ".obj"));

    // ------------------------------------------
    // Load game start
    // ------------------------------------------

    // Setup background music
    audio::MusicPlayer::Get().Initialize(game->getMusicGroup());
    audio::MusicPlayer::Get().SetRestartOnTransition(true); // Restart tracks from beginning on transition

    // Initialize behavior system
    BehaviorRegistry::initialize();
    AttackActionRegistry::initialize();
    MoveActionRegistry::initialize();
    
    // load levels
    Enemy::generateTemplates(game);
    SingleSide::generateTemplates(game);
    PaperMesh::generateTemplates(game);
    Paper::generateTemplates(game);

    // initialize menus (menu scene is ready)
    game->initPaperView();
    game->initMenus();

    while (game->getEngine()->isRunning()) {
        game->update(game->getEngine()->getDeltaTime());
    }
    
    delete game;
}