#ifndef BOSS_H
#define BOSS_H

#include "util/includes.h"

class Game;
class PaperView;

class Boss {
private:
    Game* game;
    PaperView* paperView;
    
    // Health
    int health;
    int maxHealth;
    
    // State
    std::string stage;
    bool vulnerable;
    
    // Hitbox
    float hitboxRadius;
    vec2 hitboxPosition;
    
    // Hand position in 2D paper plane coordinates
    vec2 hand2DPosition = vec2(0.0f, 0.0f);  // 2D position on paper plane (bounded -12 to 12, -4.5 to 4.5)
    float handHeight = 0.0f;  // Height offset along plane normal (for raising/lowering)
    
    // Boss node animation
    float bossAnimationTime = 0.0f;  // Time for back-and-forth animation
    float bossAnimationSpeed = 1.0f;  // Speed of boss sliding animation
    
    // Vulnerability animation state
    enum class VulnerableState {
        None,        // Not vulnerable, normal sliding
        Lowering,    // Lowering toward paper
        Lowered,     // Lowered and vulnerable
        Raising      // Raising back up
    };
    VulnerableState vulnerableState = VulnerableState::None;
    float vulnerableLowerProgress = 0.0f;  // 0.0 to 1.0, progress through lowering animation
    float vulnerableRaiseProgress = 0.0f;  // 0.0 to 1.0, progress through raising animation
    float vulnerableLowerDuration = 0.3f;  // Duration of lowering animation in seconds
    float vulnerableRaiseDuration = 0.3f;  // Duration of raising animation in seconds
    float vulnerableRaisedHeight = 0.0f;   // Height when raised
    float vulnerableLoweredHeight = 0.0f;  // Height when lowered
    
    // Vulnerability recovery
    float vulnerableTime = 0.0f;  // Time spent in lowered state
    float vulnerableDuration = 2.0f;  // How long to stay lowered before raising again
    
    // Scale for the hand (longer/taller - extended along finger direction)
    glm::vec3 handScale = glm::vec3(0.32f, 0.72f, 0.24f);  // Extended in Y direction (finger direction, 20% smaller)
    
    // Random vulnerability timing
    float timeSinceLastVulnerable = 0.0f;
    float minTimeBetweenVulnerable = 2.0f;  // Minimum seconds between vulnerable periods
    float maxTimeBetweenVulnerable = 5.0f;  // Maximum seconds between vulnerable periods
    float nextVulnerableTime = 0.0f;  // When to trigger next vulnerable period
    
    // Invincibility frames
    float iframeTimer = 0.0f;  // Time remaining for invincibility frames
    float iframeDuration = 0.3f;  // Duration of invincibility frames after taking damage

public:
    Boss(Game* game, PaperView* paperView);
    ~Boss();
    
    // Getters
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    std::string getStage() const { return stage; }
    bool isVulnerable() const { return vulnerable; }
    float getHitboxRadius() const { return hitboxRadius; }
    vec2 getHitboxPosition() const { return hitboxPosition; }
    Node* getBossNode() const;  // Get the boss node from paperView (the hand itself)
    
    // Setters
    void setHealth(int health) { this->health = health; }
    void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }
    void setStage(const std::string& stage) { this->stage = stage; }
    void setVulnerable(bool vulnerable);
    void setHitboxRadius(float radius) { this->hitboxRadius = radius; }
    void setHitboxPosition(const vec2& position) { this->hitboxPosition = position; }
    
    // Update
    void update(float dt);
    
    // Vulnerability animation
    void startVulnerable();  // Start the vulnerable lowering animation

    vec2 get2DPosition() const;  // Get 2D position for enemy spawn/damage zone location
    void spawnEnemy();
    void attack();
    
    // Damage
    void onDamage(int damage);
    bool isDead() const { return health <= 0; }
    
    // Helper function to clamp position to maintain constant height above plane
    // If preserveDistance is true, preserves the distance along normal; otherwise uses baseDistance
    glm::vec3 clampToPlaneHeight(const glm::vec3& position, bool preserveDistance = false) const;
};

#endif