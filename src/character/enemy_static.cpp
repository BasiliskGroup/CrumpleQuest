#include "character/enemy.h"
#include "game/game.h"
#include "levels/levels.h"
#include "weapon/weapon.h"


std::unordered_map<std::string, std::function<Enemy*(vec2, SingleSide*)>> Enemy::templates;

void Enemy::generateTemplates(Game* game) {

    // notebook
    templates["glue"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 0.75, 0.75 }, .collider=side->getCollider("quad") });
        Enemy* enemy = new Enemy(game, 3, 1, node, side, nullptr, nullptr, 0.4, { 0.75, 0.75 });
        enemy->idleAnimation = game->getAnimation("player_idle");
        enemy->runAnimation = game->getAnimation("player_run");
        enemy->attackAnimation = game->getAnimation("player_attack");
        enemy->setWeapon(new ProjectileWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 },
            0 // ricochet
        ));
        return enemy;
    };

    templates["staple"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 1.0, 1.0 }, .collider=side->getCollider("quad") });
        Enemy* enemy = new Enemy(game, 3, 2, node, side, nullptr, nullptr, 0.5, { 1.0, 1.0 });
        enemy->idleAnimation = game->getAnimation("player_idle");
        enemy->runAnimation = game->getAnimation("player_run");
        enemy->attackAnimation = game->getAnimation("player_attack");
        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 },
            50.0f // knockback
        ));
        return enemy;
    };

    templates["clipfly"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 0.3, 0.3 }, .density=0.1, .collider=side->getCollider("quad") });
        Enemy* enemy = new Enemy(game, 3, 4, node, side, nullptr, nullptr, 0.15, { 0.3, 0.3 });
        enemy->idleAnimation = game->getAnimation("player_idle");
        enemy->runAnimation = game->getAnimation("player_run");
        enemy->attackAnimation = game->getAnimation("player_attack");
        enemy->setWeapon(new ContactWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 }
        ));
        return enemy;
    };
}