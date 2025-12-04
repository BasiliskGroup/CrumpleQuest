#include "character/enemy.h"
#include "game/game.h"
#include "levels/levels.h"
#include "weapon/weapon.h"


std::unordered_map<std::string, std::function<Enemy*(vec2, SingleSide*)>> Enemy::templates;

void Enemy::generateTemplates(Game* game) {

    // notebook
    templates["glue"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 1.8, 1.8 }, .collider=side->getCollider("quad"), .colliderScale={0.5, 0.9}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 1, node, side, nullptr, nullptr, 0.4, node->getScale(), "hit-glue");
        enemy->idleAnimation = game->getAnimation("glue_idle");
        enemy->runAnimation = game->getAnimation("glue_idle");
        enemy->attackAnimation = game->getAnimation("glue_attack");
        enemy->setWeapon(new ProjectileWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .speed=10, .radius=0.25 },
            3,
            0 // ricochet
        ));
        return enemy;
    };

    templates["staple"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 2.0, 2.0 }, .collider=side->getCollider("quad"), .colliderScale={0.7, 0.7}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 2, node, side, nullptr, nullptr, 0.5, node->getScale(), "hit-staple-remover");
        enemy->idleAnimation = game->getAnimation("staple_idle");
        enemy->runAnimation = game->getAnimation("staple_idle");
        enemy->attackAnimation = game->getAnimation("staple_attack");
        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 },
            2,
            50.0f // knockback
        ));
        return enemy;
    };

    templates["clipfly"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 1.5, 1.5 }, .collider=side->getCollider("quad"), .colliderScale={0.6, 0.6}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 4, node, side, nullptr, nullptr, 0.15, node->getScale(), "hit-clipfly");
        enemy->idleAnimation = game->getAnimation("clipfly_idle");
        enemy->runAnimation = game->getAnimation("clipfly_idle");
        enemy->attackAnimation = game->getAnimation("clipfly_attack");
        enemy->setWeapon(new ContactWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 }
        ));
        return enemy;
    };

    templates["integral"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 1.5, 1.5 }, .collider=side->getCollider("quad"), .colliderScale={0.6, 0.6}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 4, node, side, nullptr, nullptr, 0.15, node->getScale(), "hit-clipfly");
        enemy->idleAnimation = game->getAnimation("integral_idle");
        enemy->runAnimation = game->getAnimation("integral_idle");
        enemy->attackAnimation = game->getAnimation("integral_attack");
        enemy->setWeapon(new ContactWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 }
        ));
        return enemy;
    };

    templates["sigma"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 2.0, 2.0 }, .collider=side->getCollider("quad"), .colliderScale={0.7, 0.7}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 2, node, side, nullptr, nullptr, 0.5, node->getScale(), "hit-staple-remover");
        enemy->idleAnimation = game->getAnimation("sigma_idle");
        enemy->runAnimation = game->getAnimation("sigma_idle");
        enemy->attackAnimation = game->getAnimation("sigma_attack");
        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 },
            50.0f // knockback
        ));
        return enemy;
    };

    templates["pi"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .position=pos, .scale={ 1.8, 1.8 }, .collider=side->getCollider("quad"), .colliderScale={0.5, 0.9}, .density=0.01 });
        Enemy* enemy = new Enemy(game, 3, 1, node, side, nullptr, nullptr, 0.4, node->getScale(), "hit-glue");
        enemy->idleAnimation = game->getAnimation("pi_idle");
        enemy->runAnimation = game->getAnimation("pi_idle");
        enemy->attackAnimation = game->getAnimation("pi_attack");
        enemy->setWeapon(new ProjectileWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={ 0.25, 0.25 } }, 
            { .damage=1, .life=5.0f, .radius=0.25 },
            0 // ricochet
        ));
        return enemy;
    };

}