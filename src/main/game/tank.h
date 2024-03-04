#pragma once
#include "../wisteria_engine/controlledscene3d.h"
#include "../wisteria_engine/gameobject3d.h"
#include "../wisteria_engine/assets.h"
#include "../wisteria_engine/camera.h"
#include "helpers.h"

using namespace engine;

#define TANK_RIGID 0
#define TANK_SMOOTH 1

#define TANK_ACTION_FORWARD 1
#define TANK_ACTION_BACKWARD 2
#define TANK_ACTION_CANNON_UP 3
#define TANK_ACTION_LEFT 4
#define TANK_ACTION_RIGHT 8
#define TANK_ACTION_CANNON_DOWN 12
#define TANK_ACTION_TURRET_LEFT 16
#define TANK_ACTION_TURRET_RIGHT 32
#define TANK_ACTION_FIRE 48
#define TANK_ACTION_MAX 63

namespace game
{
    class Game;
    class Cannonball;
    class Tank : public GameObject
    {
    public:
        Tank(glm::vec3 pos, glm::vec3 scale = glm::vec3(1), glm::quat rot = QUAT1);
        ~Tank() = default;
        
        float speed = 10.0f;
        float angularSpeed = 55.0f;
        float turretSpeed = 60.0f;
        float cannonSpeed = 45.0f;
        bool collisionMode = TANK_SMOOTH;
        GameObject *left_track, *right_track;
        GameObject *turret;
        GameObject *cannon;

        static const float collisionRadius;
        static const float cannonLength;
        static const std::vector<std::pair<float, float>> tankHues;

        void OnCollision(const SphereBoxCollisionEvent &event) override;
        void OnCollision(const SphereSphereCollisionEvent &event) override;

        void SetHueVariation(int hueIndex);
        void SetFollowCamera(Camera *camera) { followCamera = camera; }
        void Fire() const;

        // move logic
        void Update(float deltaTime);

    private:
        static void Init();
        static bool initialized;

        void ReactOverlap(glm::vec3 dirToContact, float distance);
        void OnCollisionTank(const SphereSphereCollisionEvent &collision);
        void OnCollisionWall(const SphereBoxCollisionEvent &collision);
        void OnHit();

        Camera *followCamera = nullptr;
        int hp = 3;
        int hueIndex = 0;

        float nextActionIn = 0.0f;
        uint8_t currentAction = 0;
    };

    class Cannonball : public GameObject
    {
    public:
        Cannonball(const Tank *tank);
        ~Cannonball() = default;

        void OnCollision(const SphereBoxCollisionEvent &event) override;
        void OnCollision(const SphereSphereCollisionEvent &event) override;
        void OnTransformChange() override;

        const Tank *sourceTank;

        static float initialSpeed;
        static float gravity;
    };
}

#include "game.h"