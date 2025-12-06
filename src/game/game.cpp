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
    playerAnimator(nullptr),
    menuScene(nullptr),
    menuCamera(nullptr),
    musicGroup(0),
    sfxGroup(0)
{
    // Floor will be created in startGame() after Paper templates are generated
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
    
    // Initialize singletons (Meyers singletons are created on first Get() call)
    audio::SFXPlayer::Get().Initialize(sfxGroup);
    MenuManager::Get().SetGame(this);
    
    // Create menu scene
    menuScene = new Scene2D(engine);
    menuCamera = new StaticCamera2D(engine);
    menuCamera->setScale(12.0f, 9.0f);  // Match gameplay camera scale
    menuScene->setCamera(menuCamera);
    menuScene->getSolver()->setGravity(0);
}

Game::~Game() {
    delete player; player = nullptr;
    // Floor owns all Paper instances, so delete floor first
    delete floor; floor = nullptr;
    // Paper is now deleted by floor's destructor
    paper = nullptr;
    currentSide = nullptr;

    // shutdown audio system
    audioManager.Shutdown();
    
    // Singletons are automatically cleaned up at program exit
    
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

    // Clean up menu scene
    delete menuCamera; menuCamera = nullptr;
    delete menuScene; menuScene = nullptr;

    // scene2D will handle deletion
    uiElements.clear();

    // basilisk closing, must be last
    delete engine; engine = nullptr;
}

void Game::update(float dt) {
    // Update elapsed time for sound cooldowns
    elapsedTime += dt;
    
    // Process pending return to main menu (deferred from button callback)
    if (pendingReturnToMainMenu) {
        pendingReturnToMainMenu = false;
        processReturnToMainMenu();
        return; // Skip rest of update this frame
    }

    // pathing
    pathTimer -= dt;
    if (paper && pathTimer < 0) {
        paper->updatePathing(player->getPosition());
        pathTimer = maxPathTimer;
    }

    // get mouse statewa
    bool rightIsDown = engine->getMouse()->getRightDown();
    bool leftIsDown = engine->getMouse()->getLeftDown();
    vec2 mousePos = { 0, 0 };
    
    // Use menu camera for mouse position when in menus, otherwise use game camera
    if (MenuManager::Get().hasActiveMenu()) {
        mousePos = { 
            engine->getMouse()->getWorldX(menuCamera), 
            engine->getMouse()->getWorldY(menuCamera) 
        };
    } else if (currentSide && currentSide->getScene() && currentSide->getScene()->getCamera()) {
        mousePos = { 
            engine->getMouse()->getWorldX(getScene()->getCamera()), 
            engine->getMouse()->getWorldY(getScene()->getCamera()) 
        };
        mousePos.x *= 8.0 / 4.6153;
        mousePos.y *= 4.5 / 3.492;
    }    

    
    // update menu events
    MenuManager::Get().handleEvent(mousePos, leftIsDown);

    // keyboard
    auto keys = this->engine->getKeyboard();
    if (keys->getPressed(GLFW_KEY_SPACE) && kWasDown == false) {
        if (paper) {
            bool unfolded = paper->unfold(player->getPosition());
            if (unfolded) {
                paperView->regenerateMesh();
                setSideToPaperSide();
            }
        }
    }
    kWasDown = keys->getPressed(GLFW_KEY_SPACE);
    
    // reset geometry (r key)
    if (keys->getPressed(GLFW_KEY_R) && rWasDown == false && !MenuManager::Get().hasActiveMenu()) {
        if (paper) {
            paper->resetGeometry();
            if (paperView) {
                paperView->regenerateMesh();
            }
        }
    }
    rWasDown = keys->getPressed(GLFW_KEY_R);
    
    // pause (escape key)
    if (keys->getPressed(GLFW_KEY_ESCAPE) && escapeWasDown == false) {
        if (player == nullptr) {
            if (MenuManager::Get().getMenuStackSize() > 1) {
                MenuManager::Get().popMenu();
            } else {
                MenuManager::Get().pushSettingsMenu();
            }
        } else {
            // in game
            if (MenuManager::Get().hasActiveMenu()) {
                MenuManager::Get().popMenu();
            } else {
                MenuManager::Get().pushSettingsMenu();
            }
        }
    }
    escapeWasDown = keys->getPressed(GLFW_KEY_ESCAPE);

    // Automatic room switching based on AABB boundary crossing
    // (Arrow key navigation removed - now automatic when crossing boundaries)

    // folding
    if (!MenuManager::Get().hasActiveMenu())
    {
        if (!rightWasDown && rightIsDown) { // we just clicked
            rightStartDown = mousePos;
            lastFoldMousePos = mousePos;
            
            if (paper) {
                foldIsActive = paper->activateFold(mousePos);
                
                // Play fold sound only if a fold was actually activated
                if (foldIsActive) {
                    audio::SFXPlayer::Get().Play("fold");
                }
            }
    
        } else if (rightWasDown && !rightIsDown) { // we just let go
            if (paper) {
                bool foldSucceeded = paper->fold(rightStartDown, mousePos);
                paperView->regenerateMesh();
                paper->deactivateFold();
                
                // Play fold end sound only if the fold was valid and successful
                if (foldIsActive && foldSucceeded) {
                    audio::SFXPlayer::Get().Play("fold_end");
                }
                
                foldIsActive = false;
            }
        }
    
        // continuous fold preview
        if (rightIsDown && paper) {
            // Calculate mouse velocity for sound effects
            float mouseVelocity = glm::length(mousePos - lastFoldMousePos) / dt;
            float velocityThreshold = 15.0f; // World units per second (adjusted for 2D space)
            float soundCooldown = 0.15f; // Minimum time between sounds
            
            if (mouseVelocity > velocityThreshold && (elapsedTime - lastFoldSoundTime) > soundCooldown) {
                // Scale volume based on velocity (from threshold to 2x threshold = 0.15 to 0.65)
                float normalizedVelocity = (mouseVelocity - velocityThreshold) / velocityThreshold;
                float volume = glm::clamp(0.15f + normalizedVelocity * 0.5f, 0.15f, 0.65f);
                audio::SFXPlayer::Get().PlayWithVolume("fold", volume);
                lastFoldSoundTime = elapsedTime;
            }
            
            lastFoldMousePos = mousePos;
            
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

    // Automatic room switching based on AABB boundary crossing
    if (floor && paper && player && !MenuManager::Get().hasActiveMenu()) {
        vec2 playerPos = player->getPosition();
        vec3 playerVel = player->getVelocity();
        PaperMesh* currentMesh = (paper->curSide == 0) ? paper->paperMeshes.first : paper->paperMeshes.second;
        auto aabb = currentMesh->getAABB();
        
        vec2 bl = aabb.first;
        vec2 tr = aabb.second;
        
        // Check boundaries with 1 unit padding for detection
        // Only switch if there's a room in that direction AND player is moving outwards
        // This prevents getting caught between room transitions
        // Also check if transition is already in progress to prevent multiple calls
        if (paperView && paperView->isTransitioning()) {
            // Transition already in progress, skip boundary checks
        } else if (playerPos.x < bl.x - 1.0f && playerVel.x < 0.0f) {
            // Left boundary crossed (west) and moving left
            std::cout << "[Game] Left boundary crossed, checking adjacent room..." << std::endl;
            Paper* adjacentRoom = floor->getAdjacentRoom(-1, 0);
            if (adjacentRoom) {
                std::cout << "[Game] Switching to room at (-1, 0)" << std::endl;
                paperView->switchToRoom(adjacentRoom, -1, 0);
            } else {
                std::cout << "[Game] No adjacent room found at (-1, 0)" << std::endl;
            }
        } else if (playerPos.x > tr.x + 1.0f && playerVel.x > 0.0f) {
            // Right boundary crossed (east) and moving right
            std::cout << "[Game] Right boundary crossed, checking adjacent room..." << std::endl;
            Paper* adjacentRoom = floor->getAdjacentRoom(1, 0);
            if (adjacentRoom) {
                std::cout << "[Game] Switching to room at (1, 0)" << std::endl;
                paperView->switchToRoom(adjacentRoom, 1, 0);
            } else {
                std::cout << "[Game] No adjacent room found at (1, 0)" << std::endl;
            }
        } else if (playerPos.y < bl.y - 1.0f && playerVel.y < 0.0f) {
            // Bottom boundary crossed (south, +y direction) and moving down
            std::cout << "[Game] Bottom boundary crossed, checking adjacent room..." << std::endl;
            Paper* adjacentRoom = floor->getAdjacentRoom(0, 1);
            if (adjacentRoom) {
                std::cout << "[Game] Switching to room at (0, 1)" << std::endl;
                paperView->switchToRoom(adjacentRoom, 0, 1);
            } else {
                std::cout << "[Game] No adjacent room found at (0, 1)" << std::endl;
            }
        } else if (playerPos.y > tr.y + 1.0f && playerVel.y > 0.0f) {
            // Top boundary crossed (north, -y direction) and moving up
            std::cout << "[Game] Top boundary crossed, checking adjacent room..." << std::endl;
            Paper* adjacentRoom = floor->getAdjacentRoom(0, -1);
            if (adjacentRoom) {
                std::cout << "[Game] Switching to room at (0, -1)" << std::endl;
                paperView->switchToRoom(adjacentRoom, 0, -1);
            } else {
                std::cout << "[Game] No adjacent room found at (0, -1)" << std::endl;
            }
        }
    }

    // basilisk update
    engine->update();
    
    // Update and render scenes
    if (currentSide) {
        // Always update game scene if it exists (unless menus are active)
        if (!MenuManager::Get().hasActiveMenu()) {
            if (player != nullptr) {
                currentSide->update(player->getPosition(), dt, player);
                paper->getBackSide()->update({-100, -100}, dt); // player isn't on that side
            } else {
                currentSide->update({0, 0}, dt);
                paper->getBackSide()->update({-100, -100}, dt);
            }
            
            // Check if all enemies are defeated and set isOpen
            if (paper) {
                bool wasOpen = paper->isOpen;
                paper->checkAndSetOpen();
                
                // If paper is open, reveal all valid adjacent room directions
                if (paper->isOpen && floor && paperView) {
                    bool topValid = floor->getAdjacentRoom(0, -1) != nullptr;
                    bool bottomValid = floor->getAdjacentRoom(0, 1) != nullptr;
                    bool leftValid = floor->getAdjacentRoom(1, 0) != nullptr;
                    bool rightValid = floor->getAdjacentRoom(-1, 0) != nullptr;
                    
                    // Play sound when squares first appear (room just opened)
                    if (!wasOpen && paper->isOpen) {
                        audio::SFXPlayer::Get().Play("menu_touch");
                    }
                    
                    paperView->showDirectionalNodes(topValid, bottomValid, leftValid, rightValid);
                } else if (paper && floor && paperView && !paper->isOpen) {
                    // Hide all directions if paper is not open
                    paperView->hideDirectionalNodes();
                }
            }
        }
        // Game scene is paused if menus are active, but still rendered

        // Render the level onto the paper fbo
        paperView->renderLevelFBO(paper);
        // Render the 3d view to the screen
        engine->getFrame()->use();
        paperView->update(paper);
        paperView->render();
    }
    
    if (MenuManager::Get().hasActiveMenu()) {
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
    
    // Regenerate PaperView mesh now that paper exists
    if (paperView) {
        paperView->regenerateMesh();
    }
}

void Game::initMenus() {
    MenuManager::Get().pushMainMenu();
}

void Game::initPaperView() {
    // Create paper scene
    paperView = new PaperView(this);
}

// Spawn in player, enemies, etc
void Game::startGame() {
    // Create the floor (this will generate rooms and create Paper instances)
    if (floor) {
        delete floor;
    }
    floor = new Floor(this);

    // Load minimap
    paperView->createMinimap();
    
    // Get the center room (spawn room) and set it as the current paper
    Paper* centerPaper = floor->getCenterRoom();
    if (!centerPaper) {
        std::cerr << "Error: Center room not found in floor!" << std::endl;
        return;
    }
    
    this->paper = centerPaper;
    this->currentSide = this->paper->getSingleSide();
    
    // Regenerate PaperView mesh now that paper exists
    if (paperView) {
        paperView->regenerateMesh();
    }

    // create player
    Node2D* playerNode = paper->getSingleSide()->getPlayerNode();
    Player* player = new Player(this, 3, 6, playerNode, getSide(), nullptr, 0.6f, playerNode->getScale());
    setPlayer(player);

    float meleeRadius = 1.0f;

    // create weapons
    player->setWeapon(new MeleeWeapon(player, { .mesh=getMesh("quad"), .material=getMaterial("circle"), .scale={meleeRadius, meleeRadius}}, { .damage=1, .life=0.2f, .radius=meleeRadius / 2.0f }, 0.5f, 6.0f));
}

void Game::switchToRoom(Paper* newPaper, int dx, int dy) {
    if (!newPaper || !floor || !player) {
        std::cout << "[Game] switchToRoom: Invalid parameters (newPaper=" << (newPaper ? "valid" : "null") 
                  << ", floor=" << (floor ? "valid" : "null") << ", player=" << (player ? "valid" : "null") << ")" << std::endl;
        return;
    }
    
    // Update floor's current position
    int currentX = floor->getCurrentX();
    int currentY = floor->getCurrentY();
    
    int newX = currentX + dx;
    int newY = currentY + dy;
    floor->setCurrentPosition(newX, newY);
    
    // Verify position was set correctly
    int verifyX = floor->getCurrentX();
    int verifyY = floor->getCurrentY();
    
    // Switch to the new paper
    this->paper = newPaper;
    this->currentSide = this->paper->getSingleSide();
    
    // Regenerate PaperView mesh
    if (paperView) {
        paperView->regenerateMesh();
        // Hide directions initially - they will be revealed if the room is open
        paperView->hideDirectionalNodes();
        // Update minimap to reflect new player position
        paperView->createMinimap();
    }
    
    // Update player nodes to the new room's nodes and update side reference
    setSideToPaperSide();
    player->setSide(this->currentSide);
    
    // Calculate spawn position based on entry direction using raycast
    PaperMesh* newMesh = (paper->curSide == 0) ? paper->paperMeshes.first : paper->paperMeshes.second;
    if (newMesh && (dx != 0 || dy != 0)) {
        // Get AABB center of the new room
        auto aabb = newMesh->getAABB();
        vec2 aabbCenter = (aabb.first + aabb.second) * 0.5f;
        
        // Calculate direction vector based on where player is coming from
        vec2 entryDirectionVec = vec2(-dx, dy);
        float dirLength = glm::length(entryDirectionVec);
        
        if (dirLength > 1e-6f) {
            vec2 entryDirection = entryDirectionVec / dirLength;
            
            // Cast ray from center in the entry direction to find edge intersection
            vec2 edgeIntersection = newMesh->getNearestEdgeIntersection(aabbCenter, entryDirection);
            
            // Move spawn point slightly inside from the edge (0.5 units inward)
            vec2 spawnDirection = -entryDirection; // Point inward from edge
            float spawnOffset = 0.5f;
            vec2 spawnPosition = edgeIntersection + spawnDirection * spawnOffset;
            
            player->setPosition(spawnPosition);
        } else {
            // Fallback if direction is invalid
            Node2D* playerNode = paper->getSingleSide()->getPlayerNode();
            if (playerNode) {
                player->setPosition(playerNode->getPosition());
            }
        }
    } else {
        // Fallback to default spawn point if mesh not available or no direction
        Node2D* playerNode = paper->getSingleSide()->getPlayerNode();
        if (playerNode) {
            player->setPosition(playerNode->getPosition());
        }
    }
}

void Game::returnToMainMenu() {
    // Set flag to process on next update (after button callback completes)
    pendingReturnToMainMenu = true;
}

void Game::processReturnToMainMenu() {
    // Delete player first, before paper (since player's node is in paper's scene)
    if (player) {
        delete player;
        player = nullptr;
    }
    
    // Destroy the floor (this will delete all Paper instances)
    if (floor) {
        delete floor;
        floor = nullptr;
    }
    
    // Paper is owned by floor, so it's already deleted
    paper = nullptr;
    currentSide = nullptr;
    
    // Clear all menus and return to main menu
    // Pop all existing menus (they will be deleted immediately, safe now)
    while (MenuManager::Get().getMenuStackSize() > 0) {
        MenuManager::Get().popMenu();
    }
    
    // Then push the main menu (which will be the only menu)
    MenuManager::Get().pushMainMenu();
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

void Game::setSideToPaperSide() {
    this->currentSide = paper->getSingleSide();
    this->player->getNode()->setMaterial(getMaterial("empty"));
    this->player->getWeaponNode()->setMaterial(getMaterial("empty"));
    this->player->setNodes(this->currentSide->getPlayerNode(), this->currentSide->getWeaponNode());
    this->player->setSide(this->currentSide);
}