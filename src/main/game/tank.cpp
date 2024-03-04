#include <iostream>
#include "tank.h"
#include "../wisteria_engine/assets.h"

using namespace game;
using namespace engine;

const float Tank::collisionRadius = 1.5f;
const float Tank::cannonLength = 1.28f;
// took me way too long to find out I was supposed to have different colored tanks
// I put time into extending the framework to support vertex colors from Blender, and
// I exported the models with multiple colors per object and now I can't split them
// so I'm just gonna tint them based on their color with the least hue component
const std::vector<std::pair<float, float>> Tank::tankHues = {
    // <body, turret> - hues from hsv
    {0.261f, 0.166f},
    {0.740f, 0.522f},
    {0.929f, 0.845f},
    {0.094f, 0.015f}
};

bool Tank::initialized = false;
void Tank::Init()
{
    initialized = true;
    const std::string models = PATH_JOIN(RESOURCE_PATH::MODELS, "tanks");
    Assets::LoadMesh("tank_track", models, "tank-track.fbx");
    Assets::LoadMesh("tank_base", models, "tank-base.fbx");
    Assets::LoadMesh("tank_turret", models, "tank-turret.fbx");
    Assets::LoadMesh("tank_cannon", models, "tank-cannon.fbx");
    Assets::LoadMesh("sphere", models, "sphere.fbx");
    Assets::LoadMesh("cannonball", models, "bullet.fbx");
}

Tank::Tank(glm::vec3 pos, glm::vec3 scale, glm::quat rot): GameObject(pos, scale, rot)
{
    if (!initialized) Init();

    tag = "Tank";
    mesh = Assets::meshes["tank_base"];
    left_track = this->CreateChild(Assets::meshes["tank_track"], glm::vec3(0.42f, 0, 0));
    right_track = this->CreateChild(Assets::meshes["tank_track"], glm::vec3(-0.42f, 0, 0));
    turret = this->CreateChild(Assets::meshes["tank_turret"], glm::vec3(0, 1, 0));
    cannon = turret->CreateChild(Assets::meshes["tank_cannon"], glm::vec3(0, 0.6f, 0.65f));
    for (auto &part : {(GameObject *)this, left_track, right_track, turret, cannon}) {
        part->material = Assets::materials["tankMaterial"];
        part->material.SetInt("HP", hp);
        part->material.SetFloat("ADD_HUE", 0.0f);
        part->material.SetFloat("SUB_HUE", 0.0f);
    }
    SetHueVariation(0);

    SetSphereHitArea(collisionRadius, glm::vec3(0, 1, 0));
    // GetHitArea().support->SetLocalScale(glm::vec3(collisionRadius));
    // GetHitArea().support->mesh = Assets::meshes["sphere"];
}

void Tank::SetHueVariation(int hueIndex)
{
    if (hueIndex >= tankHues.size()) {
        std::cerr << "Invalid hue index for tank. Must be less than " << tankHues.size() << "\n";
        return;
    }

    hueIndex = hueIndex;
    material.SetFloat("ADD_HUE", tankHues[hueIndex].first);
    material.SetFloat("SUB_HUE", tankHues[0].first);
    turret->material.SetFloat("ADD_HUE", tankHues[hueIndex].second);
    turret->material.SetFloat("SUB_HUE", tankHues[0].second);
}

void Tank::OnCollision(const SphereBoxCollisionEvent &collision)
{
    if (collision.distance == 0)
        return;  // ignore touch collisions

    if (collision.gameObject->tag == "Building") {
        OnCollisionWall(collision);
    }
}

void Tank::OnCollision(const SphereSphereCollisionEvent &collision)
{
    if (collision.distance == 0)
        return;  // ignore touch collisions

    if (collision.gameObject->tag == "Cannonball") {
        Cannonball *cannonball = static_cast<Cannonball *>(collision.gameObject);
        if (cannonball->sourceTank != this) OnHit();
    } else if (collision.gameObject->tag == "Tank") {
        OnCollisionTank(collision);
    }
}

void Tank::ReactOverlap(glm::vec3 dirToContact, float distance)
{
    // didn't know which way of calculating the 'push back' is better, so I used both
    // they have their pros and cons
    // rigid mode: the tank is pushed back along its forward axis, but it is much more 
    // restrictive in motion around walls, especially corners, feeling very rigid
    // smooth mode: the tank is pushed along the vector that connects the center of
    // the tank's collider with the closest point on the wall, but this causes the tank
    // to slide along walls, which feels very smooth

    glm::vec3 drawbackVector;
    if (collisionMode == TANK_SMOOTH) {
        dirToContact.y = 0;
        float drawback = collisionRadius - distance;// + 0.01f;
        drawbackVector = -dirToContact * drawback;
    } else {
        float overlap = collisionRadius - distance;
        glm::vec3 overlapDirection = dirToContact;
        float cosAngle = glm::dot(overlapDirection, forward);
        float absCosAngle = glm::abs(cosAngle);
        // completely ignore collisions realllly close to parallel
        if (absCosAngle < 0.03f) return;
        // be more admissive with collisions from the side
        float drawback;
        if (absCosAngle < 0.2f) {
            float admittedOverlap = 4.0f * (0.2f - absCosAngle);
            if (overlap < admittedOverlap) return;
            drawback = admittedOverlap / cosAngle;
        } else {
            drawback = overlap / cosAngle;
        }
        drawbackVector = -forward * drawback;
    }

    Translate(drawbackVector);
    if (followCamera) 
        followCamera->Translate(drawbackVector);
}

void Tank::OnCollisionTank(const SphereSphereCollisionEvent &collision)
{
    ReactOverlap(collision.displacement / collision.distance, 
                 collision.distance - collisionRadius);
}

void Tank::OnCollisionWall(const SphereBoxCollisionEvent &collision)
{
    ReactOverlap(collision.displacement / collision.distance, collision.distance);
}

void Tank::OnHit()
{
    hp--;
    if (hp <= 0) {
        ((Game *)scene)->RemoveTank(this);
        scene->Destroy(this);
        return;
    }
    for (auto &part : {(GameObject *)this, left_track, right_track, turret, cannon}) {
        part->material.SetInt("HP", hp);
    }
}

void Tank::Fire() const {
    Cannonball *cannonball = new Cannonball(this); 
    scene->AddToScene(cannonball);
    scene->AddToLayer(cannonball, LAYER_CANNONBALLS);
}

void Tank::Update(float deltaTime)
{
    if (nextActionIn <= 0) {
        nextActionIn = randomFloat(0.5f, 1.0f);
        currentAction = (uint8_t)randomInt(0, TANK_ACTION_MAX);
    }

    if ((currentAction & TANK_ACTION_CANNON_UP) == TANK_ACTION_CANNON_UP) {
        glm::vec3 cannonRot = glm::eulerAngles(cannon->GetLocalRotation());
        cannonRot.x += glm::radians(cannonSpeed * deltaTime);
        cannonRot.x = glm::min(cannonRot.x, glm::radians(30.0f));
        cannon->SetLocalRotation(glm::quat(cannonRot));
    } else if (currentAction & TANK_ACTION_FORWARD) {
        Translate(speed * deltaTime * GetForward());
    } else if (currentAction & TANK_ACTION_BACKWARD) {
        Translate(-speed * deltaTime * GetForward());
    }

    if ((currentAction & TANK_ACTION_CANNON_DOWN) == TANK_ACTION_CANNON_DOWN) {
        glm::vec3 cannonRot = glm::eulerAngles(cannon->GetLocalRotation());
        cannonRot.x -= glm::radians(cannonSpeed * deltaTime);
        cannonRot.x = glm::max(cannonRot.x, glm::radians(-30.0f));
        cannon->SetLocalRotation(glm::quat(cannonRot));
    } else if (currentAction & TANK_ACTION_LEFT) {
        glm::quat rot = glm::angleAxis(glm::radians(angularSpeed * deltaTime), up);
        Rotate(rot);
    } else if (currentAction & TANK_ACTION_RIGHT) {
        glm::quat rot = glm::angleAxis(glm::radians(-angularSpeed * deltaTime), up);
        Rotate(rot);
    }

    if ((currentAction & TANK_ACTION_FIRE) == TANK_ACTION_FIRE) {
        Fire();
        currentAction &= ~TANK_ACTION_FIRE;  // one-time action
    } else if (currentAction & TANK_ACTION_TURRET_LEFT) {
        glm::quat rot = glm::angleAxis(glm::radians(turretSpeed * deltaTime), up);
        turret->Rotate(rot);
    } else if (currentAction & TANK_ACTION_TURRET_RIGHT) {
        glm::quat rot = glm::angleAxis(glm::radians(-turretSpeed * deltaTime), up);
        turret->Rotate(rot);
    }
    nextActionIn -= deltaTime;
}

float Cannonball::initialSpeed = 15.0f;
float Cannonball::gravity = -9.81f;
Cannonball::Cannonball(const Tank *tank): GameObject(Assets::meshes["cannonball"], 
    tank->cannon->GetPosition() + tank->cannon->GetForward() * Tank::cannonLength)
{
    sourceTank = tank;
    tag = "Cannonball";
    velocity = tank->cannon->GetForward() * initialSpeed;
    acceleration = glm::vec3(0, gravity, 0);
    SetSphereHitArea(1);
}

void Cannonball::OnCollision(const SphereBoxCollisionEvent &event)
{
    if (event.gameObject->tag != "Building")
        return;
    scene->Destroy(this);
}

void Cannonball::OnCollision(const SphereSphereCollisionEvent &event)
{
    if (event.gameObject->tag != "Tank" || event.gameObject == sourceTank)
        return;
    scene->Destroy(this);
}

void Cannonball::OnTransformChange()
{
    if (position.y < 0) {
        scene->Destroy(this);
    }
}
