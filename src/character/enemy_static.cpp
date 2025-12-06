#include "character/enemy.h"
#include "character/moveActions.h"
#include "character/attackActions.h"
#include "game/game.h"
#include "levels/levels.h"
#include "weapon/weapon.h"

std::unordered_map<std::string, std::function<Enemy*(vec2, SingleSide*)>> Enemy::templates;

std::unordered_map<std::string, std::vector<std::pair<std::string, float>>> Enemy::enemyBiomes = {
    { "notebook", { { "glue", 4.0f }, { "staple", 3.0f }, { "clipfly", 1.0f } } },
    { "grid", { { "integral", 4.0f }, { "sigma", 1.0f }, { "pi", 3.0f } } }
};

void Enemy::generateTemplates(Game* game) {

    // notebook
    templates["glue"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 1.8, 1.8 }, .collider=side->getCollider("quad"), .colliderScale={0.5, 0.9}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 1, node, side, nullptr, nullptr, 0.4, node->getScale(), "hit-glue", 0.75f);
        enemy->idleAnimation = game->getAnimation("glue_idle");
        enemy->runAnimation = game->getAnimation("glue_idle");
        enemy->attackAnimation = game->getAnimation("glue_attack");

        float projectileRadius = 0.5f;

        enemy->setWeapon(new ProjectileWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ projectileRadius, 2.0f * projectileRadius } }, 
            { .damage=1, .life=5.0f, .speed=7.0f, .radius=projectileRadius / 2.0f },
            3,
            { "glueProjectile" },
            0 // ricochet
        ));

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            // If can attack, stay and attack (stationary)
            if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
                return BehaviorRegistry::getBehavior("Stationary");
            }
            // If have line of sight but can't attack, run away
            else if (enemy->hasLineOfSightStatus()) {
                return BehaviorRegistry::getBehavior("Runaway");
            }
            // No line of sight, chase to get in range
            else {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            }
        };

        return enemy;
    };

    templates["staple"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 2.0, 2.0 }, .collider=side->getCollider("quad"), .colliderScale={0.7, 0.7}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 2, node, side, nullptr, nullptr, 0.5, node->getScale(), "hit-staple-remover", 0.55f);
        enemy->idleAnimation = game->getAnimation("staple_idle");
        enemy->runAnimation = game->getAnimation("staple_idle");
        enemy->attackAnimation = game->getAnimation("staple_attack");

        float meleeRadius = 1.0f;

        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ meleeRadius, meleeRadius } }, 
            { .damage=1, .life=0.1f, .radius=meleeRadius / 2.0f },
            2,
            5.0f // knockback
        ));

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            if (enemy->weaponReadyStatus() || enemy->isAttackingStatus()) {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            } else {
                return BehaviorRegistry::getBehavior("Runaway");
            }
        };

        return enemy;
    };

    templates["clipfly"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 1.5, 1.5 }, .collider=side->getCollider("quad"), .colliderScale={0.6, 0.6}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 4, node, side, nullptr, nullptr, 0.5f, node->getScale(), "hit-clipfly", 0.4f);
        enemy->idleAnimation = game->getAnimation("clipfly_idle");
        enemy->runAnimation = game->getAnimation("clipfly_idle");
        enemy->attackAnimation = game->getAnimation("clipfly_attack");

        float meleeRadius = 1.0f;
        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ meleeRadius, meleeRadius } }, 
            { .damage=1, .life=0.1f, .radius=meleeRadius / 2.0f },
            4.0f,
            5.0f // knockback
        ));

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            // If weapon is ready, chase the player
            if (enemy->weaponReadyStatus()) {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            }
            // Otherwise, wander
            return BehaviorRegistry::getBehavior("Wander");
        };

        return enemy;
    };

    templates["integral"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 1.5, 1.5 }, .collider=side->getCollider("quad"), .colliderScale={0.6, 0.6}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 4, node, side, nullptr, nullptr, 0.35, node->getScale(), "hit-clipfly", 0.0f);
        enemy->idleAnimation = game->getAnimation("integral_idle");
        enemy->runAnimation = game->getAnimation("integral_idle");
        enemy->attackAnimation = game->getAnimation("integral_attack");

        float meleeRadius = 1.0f;

        MeleeWeapon* integralWeapon = new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ meleeRadius, meleeRadius } }, 
            { .damage=1, .life=0.1f, .radius=meleeRadius / 2.0f },
            3.0f,
            5.0f // knockback
        );
        integralWeapon->setRange(4.0f);
        enemy->setWeapon(integralWeapon);

        // Use Slide attack action - dashes forward then attacks after delay
        // Parameters: dashSpeed (velocity magnitude for dash), attackTimeOffset (seconds before attack)
        enemy->setAttackAction(new SlideAttackAction(10.0f, 0.5f));
        
        enemy->setMoveAction(new JumpMoveAction(1.5f, 10.0f));
        
        float cooldownTime = 1.5f;
        int numFrames = 4;
        float frameRate = static_cast<float>(numFrames) / cooldownTime;
        enemy->getAnimator()->setFrameRate(frameRate);

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            if (enemy->weaponReadyStatus() || enemy->isAttackingStatus()) {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            } else {
                return BehaviorRegistry::getBehavior("Runaway");
            }
        };
        return enemy;
    };

    templates["sigma"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 2.0, 2.0 }, .collider=side->getCollider("quad"), .colliderScale={0.7, 0.7}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 2, node, side, nullptr, nullptr, 0.5, node->getScale(), "hit-staple-remover", 0.55f);
        enemy->idleAnimation = game->getAnimation("sigma_idle");
        enemy->runAnimation = game->getAnimation("sigma_idle");
        enemy->attackAnimation = game->getAnimation("sigma_attack");

        float meleeRadius = 1.0f;

        enemy->setWeapon(new MeleeWeapon(
            enemy, 
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ meleeRadius, meleeRadius } }, 
            { .damage=1, .life=0.1f, .radius=meleeRadius / 2.0f },
            2.0f,
            5.0f // knockback
        ));

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            if (enemy->weaponReadyStatus() || enemy->isAttackingStatus()) {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            } else {
                return BehaviorRegistry::getBehavior("Runaway");
            }
        };
        return enemy;
    };

    templates["pi"] = [game](vec2 pos, SingleSide* side) {
        Node2D* node = new Node2D(side->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position=pos, .scale={ 1.8, 1.8 }, .collider=side->getCollider("quad"), .colliderScale={0.5, 0.9}, .density=0.01, .collisionIgnoreGroups={"Character"} });
        Enemy* enemy = new Enemy(game, 3, 1, node, side, nullptr, nullptr, 0.4, node->getScale(), "hit-glue", 0.5f);
        enemy->idleAnimation = game->getAnimation("pi_idle");
        enemy->runAnimation = game->getAnimation("pi_idle");
        enemy->attackAnimation = game->getAnimation("pi_attack");

        float projectileRadius = 0.4f;

        Weapon* piWeapon = new ProjectileWeapon(
            enemy,  
            { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={ 1.5f * projectileRadius, 1.5f * projectileRadius } }, 
            { .damage=1, .life=5.0f, .speed=4.0f, .radius=projectileRadius / 2.0f },
            2.0f,
            { "piProjectile1", "piProjectile2", "piProjectile3", "piProjectile4", "piProjectile5" },
            // { "empty" },
            0 // ricochet
        );
        enemy->setWeapon(piWeapon);

        // Use Burst attack action for pi: fires 5 projectiles in quick succession
        enemy->setAttackAction("Burst");

        enemy->getBehaviorSelector() = [enemy](const vec2&, float) -> Behavior* {
            // If can attack, stay and attack (stationary)
            if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
                return BehaviorRegistry::getBehavior("Stationary");
            }
            // If have line of sight but can't attack, run away
            else if (enemy->hasLineOfSightStatus()) {
                return BehaviorRegistry::getBehavior("Runaway");
            }
            // No line of sight, chase to get in range
            else {
                return BehaviorRegistry::getBehavior("ChasePlayer");
            }
        };
        return enemy;
    };

}