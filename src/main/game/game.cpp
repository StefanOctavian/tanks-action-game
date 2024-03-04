#include <iostream>
#include "components/transform.h"
#include "../wisteria_engine/assets.h"
#include "../wisteria_engine/transform3d.h"
#include "game.h"
#include "helpers.h"
#include "tank.h"

using namespace engine;
using namespace game;

#define MAP_SIZE 100
#define MINIMAP_SIZE 40
#define MAP_SCALE 250
#define BG_COLOR glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)
#define GROUND_COLOR glm::vec3(0.749f, 0.678f, 0.639f)
#define CAMERA_MOUSE_SENSITIVITY 0.004f
#define TURRET_MOUSE_SENSITIVITY 0.004f
#define CANNON_MOUSE_SENSITIVITY 0.003f

// initialize engine-independent members
Game::Game()
{
    std::cout << "Game::Game()\n";
    srand((unsigned int)time(NULL));
    enemyTanks.reserve(4);
    buildings.reserve(20);
}

Game::~Game()
{
}

void Game::Initialize()
{
    clearColor = BG_COLOR;
    mainCamera->SetPosition(glm::vec3(0, 1.6, -mainCameraDistance));
    miniMapCamera = new Camera(glm::vec3(0, 100, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
    miniMapCamera->SetOrthographic(-MINIMAP_SIZE, MINIMAP_SIZE, -MINIMAP_SIZE, MINIMAP_SIZE, 0.1f, 1000.0f);
    miniMapCamera->SetViewport(0.05f, 0.05f, 0.3f, 0.3f);
    // miniMapCamera->active = false;
    // cameras.push_back(miniMapCamera);
    // window->DisablePointer();

    const std::string shaders = PATH_JOIN(SOURCE_PATH::MAIN, "game", "shaders");
    Assets::AddPath("Tank.VS", PATH_JOIN(shaders, "Tank.VS.glsl"));
    Assets::AddPath("Tank.FS", PATH_JOIN(shaders, "Tank.FS.glsl"));
    Assets::LoadShader("DeformTank", "Tank.VS", "Tank.FS"); 

    Assets::CreateMaterial("tankMaterial", "DeformTank");
    Assets::CreateMaterial("plainColor", "PlainColor");
    Assets::CreateMaterial("textured", "Texture");
    Assets::CreateMaterial("transformTexture", "TransformTexture");

    const std::string models = PATH_JOIN(RESOURCE_PATH::MODELS, "tanks");
    Assets::LoadMesh("ground", models, "ground.fbx");
    Assets::LoadMesh("building", models, "block.fbx");
    Assets::LoadMesh("skycube", models, "skycube2.fbx");

    const std::string textures = PATH_JOIN(RESOURCE_PATH::TEXTURES, "tanks");
    Assets::LoadTexture("ground", textures, "sandstone.jpg");
    Assets::LoadTexture("block1", textures, "blocks1.jpg");
    Assets::LoadTexture("block2", textures, "blocks2.jpg");
    Assets::LoadTexture("block3", textures, "blocks3.jpg");
    Assets::LoadTexture("block4", textures, "blocks4.jpg");
    Assets::LoadTexture("skybox", textures, "skybox2.jpg");

    collisionMasks[LAYER_TANKS] = (1 << LAYER_BUILDINGS) | (1 << LAYER_TANKS) | (1 << LAYER_CANNONBALLS);
    collisionMasks[LAYER_BUILDINGS] = (1 << LAYER_TANKS) | (1 << LAYER_CANNONBALLS);
    collisionMasks[LAYER_CANNONBALLS] = (1 << LAYER_BUILDINGS) | (1 << LAYER_TANKS);
    SetupScene();
}

void Game::SetupScene()
{
    GameObject *plane = new GameObject(Assets::meshes["ground"],
                                       glm::vec3(0), glm::vec3(MAP_SCALE, 0, MAP_SCALE));
    plane->material = Assets::materials["textured"];
    plane->material.texture = Assets::textures["ground"];

    GameObject *skybox = new GameObject(Assets::meshes["skycube"], glm::vec3(0), glm::vec3(MAP_SCALE));
    skybox->material = Assets::materials["textured"];
    skybox->material.texture = Assets::textures["skybox"];
    AddToScene(skybox);

    playerTank = new Tank(glm::vec3(0));
    playerTank->SetFollowCamera(mainCamera);
    playerTank->name = "PlayerTank";

    AddLocks();
    AddBuildings();
    AddTanks();

    AddToScene(plane);
    AddToScene(playerTank);
    AddToLayer(playerTank, LAYER_TANKS);
}

// add a world 'lock in' to avoid the tank from getting out of the map
void Game::AddLocks()
{
    GameObject *northWall = new GameObject(glm::vec3(0, 0, MAP_SIZE + 1));
    GameObject *southWall = new GameObject(glm::vec3(0, 0, -MAP_SIZE - 1));
    GameObject *eastWall  = new GameObject(glm::vec3(MAP_SIZE + 1, 0, 0));
    GameObject *westWall  = new GameObject(glm::vec3(-MAP_SIZE - 1, 0, 0));
    northWall->SetBoxHitArea(2 * MAP_SIZE, 1, 2);
    southWall->SetBoxHitArea(2 * MAP_SIZE, 1, 2);
    eastWall->SetBoxHitArea(2, 1, 2 * MAP_SIZE);
    westWall->SetBoxHitArea(2, 1, 2 * MAP_SIZE);
    for (auto &wall : {northWall, southWall, eastWall, westWall}) {
        wall->tag = "Building";
        AddToScene(wall);
        AddToLayer(wall, LAYER_BUILDINGS);
    }
}

void Game::AddTanks()
{
    for (int i = 0; i < 3; i++) {
        glm::vec3 pos;
        bool canNotPlace;
        do {
            canNotPlace = false;
            float x = randomFloat(-MAP_SIZE, MAP_SIZE);
            float z = randomFloat(-MAP_SIZE, MAP_SIZE);
            pos = glm::vec3(x, 0, z);

            for (auto &building : buildings) {
                if ((glm::abs(x - building->GetPosition().x) < building->GetLocalScale().x) &&
                    (glm::abs(z - building->GetPosition().z) < building->GetLocalScale().z))
                {
                    canNotPlace = true;
                    break;
                }
            }
        } while (canNotPlace);

        Tank *enemyTank = new Tank(pos);
        int hueIndex = randomInt(1, (int)Tank::tankHues.size() - 1);
        enemyTank->SetHueVariation(hueIndex);
        enemyTank->name = "EnemyTank";
        AddToScene(enemyTank);
        AddToLayer(enemyTank, LAYER_TANKS);
        enemyTanks.insert(enemyTank);
    }
}

void Game::RemoveTank(Tank *tank)
{
    if (tank == playerTank) {
        timeScale = 0.0f;
        std::cout << "Game over!\n";
        return;
    }
    enemyTanks.erase(tank);
}

void Game::AddBuildings()
{
    srand((unsigned int)time(NULL));
    int numBuildings = randomInt(7, 15);
    int numSmallObstacles = randomInt(5, 10);
    for (int i = 0; i < numBuildings + numSmallObstacles; i++) {
        bool isBuilding = i < numBuildings;
        float minScale = isBuilding ? 15.0f : 1.0f;
        float maxScale = isBuilding ? 30.0f : 10.0f;

        glm::vec3 pos, scale;
        bool canNotPlace;
        do {
            canNotPlace = false;
            float x = randomFloat(-MAP_SIZE, MAP_SIZE);
            float z = randomFloat(-MAP_SIZE, MAP_SIZE); 
            pos = glm::vec3(x, 0, z);

            float scaleX = randomFloat(minScale, maxScale);
            float scaleY = randomFloat(minScale, maxScale);
            float scaleZ = randomFloat(minScale, maxScale);
            scale = glm::vec3(scaleX, scaleY, scaleZ);

            // avoid spawning buildings too close to the player
            if (glm::length(pos) < scaleZ + 5) {
                canNotPlace = true;
                continue;
            }
        } while (canNotPlace);

        int textureIndex = randomInt(1, 4);
        std::string textureName = "block" + std::to_string(textureIndex);

        GameObject *building = new GameObject(Assets::meshes["building"], pos, scale);
        // building->material = Assets::materials["textured"];
        float textureScaleFactor = 3.0f * isBuilding + 1.0f;
        glm::mat3 textureScale = glm::mat3(transform::Scale(scale) / textureScaleFactor);
        building->material = Assets::materials["transformTexture"];
        building->material.texture = Assets::textures[textureName];
        building->material.SetMat3("UV_TRANSFORM", textureScale);
        building->tag = "Building";
        buildings.insert(building);

        building->SetBoxHitArea(1, 1, 1, glm::vec3(0, 0.5f, 0));
        AddToScene(building);
        AddToLayer(building, LAYER_BUILDINGS);
    }
}

void Game::Tick()
{
    for (auto &enemyTank : enemyTanks)
        enemyTank->Update(deltaTime);

    if (timer <= 0) {
        timeScale = 0.0f;
        std::cout << "Time's up!\n";
    }

    timer -= deltaTime;
}

void Game::OnInputUpdate(int mods)
{
    int moveSense = window->KeyHold(GLFW_KEY_W) - window->KeyHold(GLFW_KEY_S);
    if (moveSense != 0) {
        glm::vec3 disp = playerTank->speed * deltaTime * moveSense * playerTank->GetForward();
        playerTank->Translate(disp);
        mainCamera->Translate(disp);
        if ((glm::abs(playerTank->GetPosition().x - miniMapCamera->GetPosition().x) > minimapTargetArea.x) ||
            (glm::abs(playerTank->GetPosition().z - miniMapCamera->GetPosition().z) > minimapTargetArea.y)) {
            miniMapCamera->Translate(disp);
        }
    }

    // the positive direction of the rotation is anti-clockwise
    int rotationSense = window->KeyHold(GLFW_KEY_A) - window->KeyHold(GLFW_KEY_D);
    if (rotationSense != 0) {
        float rotationStep = glm::radians(playerTank->angularSpeed * deltaTime);
        glm::quat rot = glm::angleAxis(rotationSense * rotationStep, playerTank->GetUp());
        playerTank->Rotate(rot);
        mainCamera->RotateAround(mainCameraDistance, rotationSense * rotationStep);
    }
}

void Game::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT))
        return;

    playerTank->Fire();
}

void Game::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
        float rotationStep = CAMERA_MOUSE_SENSITIVITY * -deltaX;
        mainCamera->RotateAround(mainCameraDistance, rotationStep);
    } else {
        float rotationStep = TURRET_MOUSE_SENSITIVITY * -deltaX;
        glm::quat rot = glm::angleAxis(rotationStep, playerTank->GetUp());
        playerTank->turret->Rotate(rot);

        // int wh = windowResolution.y;
        // float normalY = 2 * glm::smoothstep(-wh / 4.0f, wh / 4.0f, wh / 2.0f - mouseY) - 1;
        // glm::vec3 cannonRot = glm::eulerAngles(playerTank->cannon->GetLocalRotation());
        // cannonRot.x = glm::mix(0.0f, glm::radians(-30.0f), normalY);
        // playerTank->cannon->SetLocalRotation(glm::quat(cannonRot));
        float rollStep = CANNON_MOUSE_SENSITIVITY * deltaY;
        glm::vec3 cannonRot = glm::eulerAngles(playerTank->cannon->GetLocalRotation());
        cannonRot.x = glm::clamp(cannonRot.x + rollStep, glm::radians(-30.0f), glm::radians(30.0f));
        playerTank->cannon->SetLocalRotation(glm::quat(cannonRot));
    }
}

void Game::OnKeyRelease(int key, int mods)
{
    if (key == GLFW_KEY_C) {
        playerTank->collisionMode = !playerTank->collisionMode;
    }
    if (key == GLFW_KEY_X) {
        window->DisablePointer();
    }
    if (key == GLFW_KEY_M) {
        // miniMapCamera->active = !miniMapCamera->active;
        if (miniMap) {
            cameras.erase(std::remove(cameras.begin(), cameras.end(), miniMapCamera), cameras.end());
            miniMap = false;
        } else {
            cameras.push_back(miniMapCamera);
            miniMap = true;
        }
    }
}