#include "levels/levels.h"
#include "ui/ui.h"
#include "resource/animation.h"
#include "resource/animator.h"
#include "weapon/weapon.h"
#include "audio/sfx_player.h"
#include <iostream>

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    currentSide(nullptr),
    paper(nullptr),
    audioManager(audio::AudioManager::GetInstance()),
    menuManager(nullptr),
    playerAnimator(nullptr),
    menuScene(nullptr),
    menuCamera(nullptr),
    musicGroup(0),
    sfxGroup(0)
{
    this->engine = new Engine(800, 450, "Crumple Quest", false);
    this->engine->setResolution(3200, 1800);
    
    // Initialize audio system
    if (!audioManager.Initialize()) {
        std::cerr << "Failed to initialize audio system\n";
    }
    
    // Create audio groups
    musicGroup = audioManager.CreateGroup("music");
    sfxGroup = audioManager.CreateGroup("sfx");
    
    // Set initial volumes
    audioManager.SetMasterVolume(1.0f);
    audioManager.SetGroupVolume(musicGroup, 0.7f);
    audioManager.SetGroupVolume(sfxGroup, 0.21f);  // 70% of max (0.7 * 0.3 = 0.21)
    
    // Create SFX player (loads all sound collections)
    sfxPlayer = std::make_unique<audio::SFXPlayer>(sfxGroup);
    
    // Create menu scene
    menuScene = new Scene2D(engine);
    menuCamera = new StaticCamera2D(engine);
    menuCamera->setScale(9.0f);
    menuScene->setCamera(menuCamera);
    menuScene->getSolver()->setGravity(0);
}

Game::~Game() {
    delete player; player = nullptr;
    delete floor; floor = nullptr;

    // shutdown audio system
    audioManager.Shutdown();

    // Delete menu manager
    delete menuManager;
    
    // materials
    for (auto const& [name, material] : materials) {
        delete material;
    }
    materials.clear();

    // images
    for (auto const& [name, image] : images) {
        delete image;
    }
    images.clear();

    // meshes
    for (auto const& [name, mesh] : meshes) {
        delete mesh;
    }
    meshes.clear();

    delete paper; paper = nullptr;
    currentSide = nullptr;

    // Clean up menu scene
    delete menuCamera; menuCamera = nullptr;
    delete menuScene; menuScene = nullptr;

    // scene2D will handle deletion
    uiElements.clear();

    // basilisk closing, must be last
    delete engine; engine = nullptr;
}

void Game::update(float dt) {
    // pathing
    pathTimer -= dt;
    if (paper && pathTimer < 0) {
        paper->updatePathing(player->getPosition());
        pathTimer = maxPathTimer;
    }

    // get mouse state
    bool rightIsDown = engine->getMouse()->getRightDown();
    bool leftIsDown = engine->getMouse()->getLeftDown();
    vec2 mousePos = { 0, 0 };
    
    // Use menu camera for mouse position when in menus, otherwise use game camera
    if (menuManager && menuManager->hasActiveMenu()) {
        mousePos = { 
            engine->getMouse()->getWorldX(menuCamera), 
            engine->getMouse()->getWorldY(menuCamera) 
        };
    } else if (currentSide && currentSide->getScene() && currentSide->getScene()->getCamera()) {
        mousePos = { 
            engine->getMouse()->getWorldX(getScene()->getCamera()), 
            engine->getMouse()->getWorldY(getScene()->getCamera()) 
        };
    }

    // update menu events
    if (menuManager) {
        menuManager->handleEvent(mousePos, leftIsDown);
    }

    // keyboard
    auto keys = this->engine->getKeyboard();
    if (keys->getPressed(GLFW_KEY_F) && kWasDown == false) {
        if (paper) {
            paper->flip();
            this->currentSide = paper->getSingleSide();
            
            // Play flip sound
            if (sfxPlayer) {
                sfxPlayer->Play("flip");
            }
        }
    }
    kWasDown = keys->getPressed(GLFW_KEY_F);
    
    // pause (escape key)
    if (keys->getPressed(GLFW_KEY_ESCAPE) && escapeWasDown == false) {
        if (player == nullptr && menuManager) {
            if (menuManager->getMenuStackSize() > 1) {
                menuManager->popMenu();
            } else {
                menuManager->pushSettingsMenu();
            }
        } else if (menuManager) {
            // in game
            if (menuManager->hasActiveMenu()) {
                menuManager->popMenu();
            } else {
                menuManager->pushSettingsMenu();
            }
        }
    }
    escapeWasDown = keys->getPressed(GLFW_KEY_ESCAPE);

    // folding
    if (menuManager && !menuManager->hasActiveMenu())
    {
        if (!rightWasDown && rightIsDown) { // we just clicked
            rightStartDown = mousePos;
            
            if (paper) {
                foldIsActive = paper->activateFold(mousePos);
                
                // Play fold sound only if a fold was actually activated
                if (sfxPlayer && foldIsActive) {
                    sfxPlayer->Play("fold");
                }
            }
    
        } else if (rightWasDown && !rightIsDown) { // we just let go
            if (paper) {
                paper->fold(rightStartDown, mousePos);
                paper->deactivateFold();
                
                // Play fold end sound only if there was an active fold
                if (sfxPlayer && foldIsActive) {
                    sfxPlayer->Play("fold_end");
                }
                
                foldIsActive = false;
            }
        }
    
        // continuous fold preview
        if (rightIsDown && paper) {
            paper->dotData();  // Update debug visualization first
            paper->previewFold(rightStartDown, mousePos);  // Show fold preview
        }
    }

    // update buttons
    for (UIElement* elem : uiElements) {
        elem->event(mousePos, leftIsDown);
    }

    rightWasDown = rightIsDown;

    // entity update
    if (player != nullptr) {
        player->move(dt);
    }

    // basilisk update
    engine->update();
    
    // Update and render scenes
    if (currentSide) {
        // Always update game scene if it exists (unless menus are active)
        if (!menuManager || !menuManager->hasActiveMenu()) {
            if (player != nullptr) {
                currentSide->update(player->getPosition(), dt);
            } else {
                currentSide->update({0, 0}, dt);
            }
        }
        // Game scene is paused if menus are active, but still rendered
        currentSide->getScene()->render();
    }
    
    if (menuManager && menuManager->hasActiveMenu()) {
        // Menu scene renders on top
        menuScene->update();
        menuScene->render();
    }
    
    engine->render();
}

void Game::setPaper(std::string str) { 
    this->paper = Paper::templates[str]();
    this->currentSide = this->paper->getSingleSide();
    this->paper->setGame(this);
}

void Game::initMenus() {
    menuManager = new MenuManager(this);
}

// Spawn in player, enemies, etc
void Game::startGame() {
    // Create the game paper and switch to it
    setPaper("empty");
    paper->regenerateWalls();

    // create player
    Node2D* playerNode = new Node2D(getScene(), { .mesh=getMesh("quad"), .material=getMaterial("knight"), .scale={1.5, 1.5}, .collider=getCollider("quad"), .colliderScale={0.5, 0.8} });
    Player* player = new Player(this, 3, 3, playerNode, getSide(), nullptr, 1.25, playerNode->getScale(), menuManager);
    setPlayer(player);

    // create weapons
    player->setWeapon(new MeleeWeapon(player, { .mesh=getMesh("quad"), .material=getMaterial("sword"), .scale={0.75, 0.75}}, { .damage=1, .life=0.2f, .radius=0.5 }, 30.0f));
}

void Game::addAnimation(std::string name, std::string folder, unsigned int nImages) {
    
    std::vector<Material*> frames;

    for (unsigned int imageIndex = 1; imageIndex <= nImages; imageIndex++) {
        std::string indivName = name + "_" + std::to_string(imageIndex);
        addImage(indivName, new Image(folder + std::to_string(imageIndex) + ".PNG"));
        addMaterial(indivName, new Material({ 1, 1, 1 }, getImage(indivName)));
        frames.push_back(getMaterial(indivName));
        std::cout << folder + std::to_string(imageIndex) + ".PNG" << std::endl;
    }

    Animation* animation = new Animation(frames);
    animations[name] = animation;
}