#pragma once
#include "components/simple_scene.h"

#include "../wisteria_engine/controlledscene3d.h"
#include "tank.h"

using namespace engine;

namespace game
{
    class Game : public ControlledScene3D
    {
    public:
        Game();
        ~Game();

        void RemoveTank(Tank *tank);

    private:
        void Initialize() override;
        void Tick() override;

        void OnInputUpdate(int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnKeyRelease(int key, int mods) override;

        void SetupScene();
        void AddLocks();
        void AddBuildings();
        void AddTanks();

        // cameras
        float mainCameraDistance = 5.0f;
        Camera *miniMapCamera;
        bool miniMap = false;
        glm::vec2 minimapTargetArea = glm::vec2(25.0f, 25.0f);

        // objects
        Tank *playerTank;
        std::unordered_set<Tank *> enemyTanks;
        std::unordered_set<GameObject *> buildings;

        // end the game after 2 minutes
        float timer = 120.0f;
    };
}