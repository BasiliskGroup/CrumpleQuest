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

    std::function<void()> refresh = [game](){game->getEngine()->update(); game->getEngine()->render();};

    // image and material
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey", "test", "knight", "table", "sword", "gun", "bullet", "wand", "green", "red", "black", "empty", "yellow", "rug_desaturated", "john", "blue", "darkred", "circle" };
    for (std::string& name : imageNames) {
        refresh();
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }
    
    // Load notebook background from art/assets
    refresh();
    game->addImage("notebook", new Image("art/assets/notebook.PNG"));
    game->addMaterial("notebook", new Material({ 1, 1, 1 }, game->getImage("notebook")));

    // Load Hands
    refresh();
    game->addImage("hand_up", new Image("art/assets/hands/up.PNG"));
    game->addMaterial("hand_up", new Material({ 1, 1, 1 }, game->getImage("hand_up")));
    game->addImage("hand_down", new Image("art/assets/hands/down.PNG"));
    game->addMaterial("hand_down", new Material({ 1, 1, 1 }, game->getImage("hand_down")));
    game->addImage("hand_left", new Image("art/assets/hands/left.PNG"));
    game->addMaterial("hand_left", new Material({ 1, 1, 1 }, game->getImage("hand_left")));
    game->addImage("hand_right", new Image("art/assets/hands/right.PNG"));
    game->addMaterial("hand_right", new Material({ 1, 1, 1 }, game->getImage("hand_right")));

    // Load ladder image from art/assets
    refresh();
    game->addImage("ladder", new Image("art/assets/ladder.PNG"));
    game->addMaterial("ladder", new Material({ 1, 1, 1 }, game->getImage("ladder")));

    // Crosshair
    game->addImage("crosshair", new Image("art/assets/crosshair.PNG"));
    game->addMaterial("crosshair", new Material({ 1, 1, 1 }, game->getImage("crosshair")));
    
    // Load menu paper background from art/assets
    refresh();
    game->addImage("menuPaper", new Image("art/assets/menuPaper.PNG"));
    game->addMaterial("menuPaper", new Material({ 1, 1, 1 }, game->getImage("menuPaper")));
    
    std::cout << "5" << std::endl;

    // Load menu button images from art/assets/buttons
    refresh();
    game->addImage("startButton", new Image("art/assets/buttons/start.PNG"));
    game->addMaterial("startButton", new Material({ 1, 1, 1 }, game->getImage("startButton")));
    game->addImage("startButtonHover", new Image("art/assets/buttons/start_hover.PNG"));
    game->addMaterial("startButtonHover", new Material({ 1, 1, 1 }, game->getImage("startButtonHover")));
    
    refresh();
    game->addImage("settingsButton", new Image("art/assets/buttons/settings.PNG"));
    game->addMaterial("settingsButton", new Material({ 1, 1, 1 }, game->getImage("settingsButton")));
    game->addImage("settingsButtonHover", new Image("art/assets/buttons/settings_hover.PNG"));
    game->addMaterial("settingsButtonHover", new Material({ 1, 1, 1 }, game->getImage("settingsButtonHover")));
    
    refresh();
    game->addImage("exitButton", new Image("art/assets/buttons/exit.PNG"));
    game->addMaterial("exitButton", new Material({ 1, 1, 1 }, game->getImage("exitButton")));
    game->addImage("exitButtonHover", new Image("art/assets/buttons/exit_hover.PNG"));
    game->addMaterial("exitButtonHover", new Material({ 1, 1, 1 }, game->getImage("exitButtonHover")));
    
    refresh();
    game->addImage("home", new Image("art/assets/buttons/home.PNG"));
    game->addMaterial("home", new Material({ 1, 1, 1 }, game->getImage("home")));
    game->addImage("home_hover", new Image("art/assets/buttons/home_hover.PNG"));
    game->addMaterial("home_hover", new Material({ 1, 1, 1 }, game->getImage("home_hover")));
    
    refresh();
    game->addImage("back", new Image("art/assets/buttons/back.PNG"));
    game->addMaterial("back", new Material({ 1, 1, 1 }, game->getImage("back")));
    game->addImage("back_hover", new Image("art/assets/buttons/back_hover.PNG"));
    game->addMaterial("back_hover", new Material({ 1, 1, 1 }, game->getImage("back_hover")));
    
    refresh();
    game->addImage("dead", new Image("art/assets/dead.PNG"));
    game->addMaterial("dead", new Material({ 1, 1, 1 }, game->getImage("dead")));
    
    refresh();
    game->addImage("volumeicon", new Image("art/assets/volumeicon.PNG"));
    game->addMaterial("volumeicon", new Material({ 1, 1, 1 }, game->getImage("volumeicon")));
    game->addImage("musicicon", new Image("art/assets/musicicon.PNG"));
    game->addMaterial("musicicon", new Material({ 1, 1, 1 }, game->getImage("musicicon")));
    game->addImage("SFXicon", new Image("art/assets/SFXicon.PNG"));
    game->addMaterial("SFXicon", new Material({ 1, 1, 1 }, game->getImage("SFXicon")));
    game->addImage("blackCircle", new Image("art/assets/blackCircle.PNG"));
    game->addMaterial("blackCircle", new Material({ 1, 1, 1 }, game->getImage("blackCircle")));
    
    refresh();
    std::unordered_map<std::string, std::vector<std::string>> levelNames = {
        { "notebook", { "blank", "level1", "level2", "level3", "level4", "level5", "weaponroom"} },
        { "grid", { "blank", "level1", "level2", "level3", "level4", "level5", "weaponroom"} },
        {"tutorial", { "tutorial" } }
    };
    for (auto& [name, levels] : levelNames) {
        for (std::string& level : levels) {
            refresh();
            game->addImage(name + "_" + level, new Image("art/maps/" + name + "/" + level + ".PNG"));
            game->addMaterial(name + "_" + level, new Material({ 1, 1, 1 }, game->getImage(name + "_" + level)));
        }
    }

    std::vector<std::string> bossHandNames = { "hand_flip", "hand_grab", "hand_hover", "hand_slam" };
    for (std::string& name : bossHandNames) {
        refresh();
        game->addImage("boss_" + name, new Image("art/sprites/enemies/boss/hands/" + name + ".PNG"));
        game->addMaterial("boss_" + name, new Material({ 1, 1, 1 }, game->getImage("boss_" + name)));
    }

    for (int i = 1; i < 6; i++) {
        refresh();
        game->addImage("piProjectile" + std::to_string(i), new Image("art/sprites/enemies/ranged/grid_pi/projectiles_pi/" + std::to_string(i) + ".PNG"));
        game->addMaterial("piProjectile" + std::to_string(i), new Material({ 1, 1, 1 }, game->getImage("piProjectile" + std::to_string(i))));
    }

    refresh();
    game->addImage("glueProjectile", new Image("art/sprites/enemies/ranged/notebook_glue/projectile_glue.PNG"));
    game->addMaterial("glueProjectile", new Material({ 1, 1, 1 }, game->getImage("glueProjectile")));
    game->addImage("stapleProjectile", new Image("art/sprites/player/weapons/stapler/bullet.png"));
    game->addMaterial("stapleProjectile", new Material({ 1, 1, 1 }, game->getImage("stapleProjectile")));

    // Player
    refresh();
    game->addAnimation("player_idle", "art/sprites/player/idle/", 4);
    game->addAnimation("player_run", "art/sprites/player/run/", 3);
    game->addAnimation("player_attack_pencil", "art/sprites/player/attack/attack_pencil/", 4);
    game->addAnimation("player_attack_gun", "art/sprites/player/attack/attack_gun/", 2);
    game->addAnimation("player_hurt", "art/sprites/player/hurt/", 3);
    // Pencil
    refresh();
    game->addAnimation("pencil_idle", "art/sprites/player/weapons/pencil/idle/", 4);
    game->addAnimation("pencil_run", "art/sprites/player/weapons/pencil/run/", 3);
    game->addAnimation("pencil_attack", "art/sprites/player/weapons/pencil/attack/", 4);
    // gun
    refresh();
    game->addAnimation("gun_idle", "art/sprites/player/weapons/stapler/idle/", 4);
    game->addAnimation("gun_run", "art/sprites/player/weapons/stapler/run/", 3);
    game->addAnimation("gun_attack", "art/sprites/player/weapons/stapler/attack/", 2);
    // Clipfly
    refresh();
    game->addAnimation("clipfly_idle", "art/sprites/enemies/contact/notebook_clipfly/idle/", 4);
    game->addAnimation("clipfly_attack", "art/sprites/enemies/contact/notebook_clipfly/attack/", 4);
    // Staple
    refresh();
    game->addAnimation("staple_idle", "art/sprites/enemies/melee/notebook_staple/idle/", 4);
    game->addAnimation("staple_attack", "art/sprites/enemies/melee/notebook_staple/attack/", 6);
    // Glue
    refresh();
    game->addAnimation("glue_idle", "art/sprites/enemies/ranged/notebook_glue/idle/", 6);
    game->addAnimation("glue_attack", "art/sprites/enemies/ranged/notebook_glue/attack/", 7);
    // Integral
    refresh();
    game->addAnimation("integral_idle", "art/sprites/enemies/contact/grid_integral/idle/", 4);
    game->addAnimation("integral_attack", "art/sprites/enemies/contact/grid_integral/attack/", 9);
    // Sigma
    refresh();
    game->addAnimation("sigma_idle", "art/sprites/enemies/melee/grid_sigma/idle/", 5);
    game->addAnimation("sigma_attack", "art/sprites/enemies/melee/grid_sigma/attack/", 6);
    // Pi
    refresh();
    game->addAnimation("pi_idle", "art/sprites/enemies/ranged/grid_pi/idle/", 6);
    game->addAnimation("pi_attack", "art/sprites/enemies/ranged/grid_pi/attack/", 7);
    // Heart
    refresh();
    game->addAnimation("heart", "art/assets/heart/", 7);
    // Staple Gun Pickup
    game->addImage("stapleGunPickup", new Image("art/sprites/player/weapons/stapler/stapler.PNG"));
    game->addMaterial("stapleGunPickup", new Material({ 1, 1, 1 }, game->getImage("stapleGunPickup")));

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper0", "paper1", "quad3D", "cube", "mug", "john", "heart", "sphere"};
    for (std::string& name : meshNames) {
        refresh();
        game->addMesh(name, new Mesh("models/" + name + ".obj")); 
    }

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
    game->initBossHealthBar();

    while (game->getEngine()->isRunning()) {
        game->update(game->getEngine()->getDeltaTime());
    }
    
    delete game;
    
    AttackActionRegistry::cleanup();
    MoveActionRegistry::cleanup();
    BehaviorRegistry::cleanup();
}