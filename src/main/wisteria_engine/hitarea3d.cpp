#include "hitarea3d.h"
#include "gameobject3d.h"
#include <iostream>

using namespace engine;

std::unordered_map<HitArea::type_pair, HitArea::CollidesFunc, HitArea::pair_hash> HitArea::collisionFuncs;

bool HitArea::Collides(HitArea *other, CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    auto collisionFunc = collisionFuncs.find({GetType(), other->GetType()});
    if (collisionFunc == collisionFuncs.end())
        return false;
    return (collisionFunc->second)(this, other, event, otherEvent);
}

void HitArea::RegisterCollisionFunc(std::type_index type1, std::type_index type2, 
                                    CollidesFunc func)
{
    collisionFuncs[{type1, type2}] = func;
}

HitArea *BoxShape::CreateHitArea(GameObject *support)
{
    return new BoxHitArea(support, *this);
}

const BoxHitArea::init BoxHitArea::initializer;
BoxHitArea::init::init()
{
    RegisterCollisionFunc(typeid(BoxHitArea), typeid(BoxHitArea), 
                          (CollidesFunc)(&CollidesBoxBox));
    RegisterCollisionFunc(typeid(BoxHitArea), typeid(SphereHitArea),
                          (CollidesFunc)(&CollidesBoxSphere));
}

bool BoxHitArea::Contains(glm::vec3 point)
{
    return point.x >= -shape.width  / 2 && point.x <= shape.width  / 2 &&
           point.y >= -shape.height / 2 && point.y <= shape.height / 2 &&
           point.z >= -shape.depth  / 2 && point.z <= shape.depth  / 2;
}

bool engine::CollidesBoxBox(BoxHitArea *box1, BoxHitArea *box2, 
                            CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    glm::vec3 center = box1->support->GetPosition();
    glm::vec3 otherCenter = box2->support->GetPosition();
    float thisW = box1->shape.width * box1->support->GetPseudoScale().x;
    float thisH = box1->shape.height * box1->support->GetPseudoScale().y;
    float thisD = box1->shape.depth * box1->support->GetPseudoScale().z;
    float otherW = box2->support->GetPseudoScale().x * box2->shape.width;
    float otherH = box2->support->GetPseudoScale().y * box2->shape.height;
    float otherD = box2->support->GetPseudoScale().z * box2->shape.depth;

    event = std::make_unique<CollisionEvent>(box2->support);
    otherEvent = std::make_unique<CollisionEvent>(box1->support);
    return center.x - thisW / 2 <= otherCenter.x + otherW / 2 &&
           center.x + thisW / 2 >= otherCenter.x - otherW / 2 && 
           center.y - thisH / 2 <= otherCenter.y + otherH / 2 &&
           center.y + thisH / 2 >= otherCenter.y - otherH / 2 &&
           center.z - thisD / 2 <= otherCenter.z + otherD / 2 &&
           center.z + thisD / 2 >= otherCenter.z - otherD / 2;
}

bool engine::CollidesBoxSphere(BoxHitArea *box, SphereHitArea *sphere, 
                               CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    glm::vec3 boxCenter = box->support->GetPosition();
    glm::vec3 sphereCenter = sphere->support->GetPosition();
    float radius = glm::abs(sphere->support->GetPseudoScale().x) * sphere->shape.radius;
    float thisW = box->shape.width * box->support->GetPseudoScale().x;
    float thisH = box->shape.height * box->support->GetPseudoScale().y;
    float thisD = box->shape.depth * box->support->GetPseudoScale().z;

    glm::vec3 closestPoint = glm::vec3(
        glm::clamp(sphereCenter.x, boxCenter.x - thisW / 2, boxCenter.x + thisW / 2),
        glm::clamp(sphereCenter.y, boxCenter.y - thisH / 2, boxCenter.y + thisH / 2),
        glm::clamp(sphereCenter.z, boxCenter.z - thisD / 2, boxCenter.z + thisD / 2));
    
    glm::vec3 displacement = sphereCenter - closestPoint;
    float distance = glm::length(displacement);
    event = std::make_unique<SphereBoxCollisionEvent>(sphere->support, closestPoint, 
                                                      displacement, distance);
    otherEvent = std::make_unique<SphereBoxCollisionEvent>(box->support, closestPoint, 
                                                           -displacement, distance);
    return distance <= radius;
}

HitArea *SphereShape::CreateHitArea(GameObject *support)
{
    return new SphereHitArea(support, *this);
}

const SphereHitArea::init SphereHitArea::initializer;
SphereHitArea::init::init()
{
    RegisterCollisionFunc(typeid(SphereHitArea), typeid(SphereHitArea), 
                          (CollidesFunc)(&CollidesSphereSphere));
    RegisterCollisionFunc(typeid(SphereHitArea), typeid(BoxHitArea), 
                          (CollidesFunc)(&CollidesSphereBox));
}

bool SphereHitArea::Contains(glm::vec3 point)
{
    return glm::distance(point, glm::vec3(0)) <= shape.radius;
}

bool engine::CollidesSphereSphere(SphereHitArea *sphere1, SphereHitArea *sphere2,
                                  CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    glm::vec3 center = sphere1->support->GetPosition();
    glm::vec3 otherCenter = sphere2->support->GetPosition();
    float radius = glm::abs(sphere1->support->GetPseudoScale().x) * sphere1->shape.radius;
    float otherRadius = glm::abs(sphere2->support->GetPseudoScale().x) * sphere2->shape.radius;

    glm::vec3 displacement = otherCenter - center;
    float distance = glm::length(displacement);
    float sumRadius = radius + otherRadius;
    event = std::make_unique<SphereSphereCollisionEvent>(sphere2->support, displacement, 
                                                        distance, sumRadius);
    otherEvent = std::make_unique<SphereSphereCollisionEvent>(sphere1->support, -displacement, 
                                                              distance, sumRadius);
    return distance <= sumRadius;
}

bool engine::CollidesSphereBox(SphereHitArea *sphere, BoxHitArea *box, 
                               CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    return CollidesBoxSphere(box, sphere, otherEvent, event);
}

void CollisionEvent::Dispatch(GameObject *target)
{
    target->OnCollision(*this);
}

void SphereBoxCollisionEvent::Dispatch(GameObject *target)
{
    target->OnCollision(*this);
}

void SphereSphereCollisionEvent::Dispatch(GameObject *target)
{
    target->OnCollision(*this);
}
