#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "hitarea3d.h"
#include "gameobject3d.h"
#include "transform3d.h"

using namespace engine;

// private constructor
GameObject::GameObject(GameObject *parent, Mesh *mesh, glm::vec3 position, 
                       glm::vec3 scale, glm::quat rotation)
{
    this->mesh = mesh;
    this->parent = parent;
    if (parent != nullptr)
        parent->children.insert(this);

    SetLocalPositionDirty(position);
    SetLocalScaleDirty(scale);
    SetLocalRotationDirty(rotation);

    RecalculateMatrix();
}

GameObject::GameObject()
{
    position = glm::vec3(0);
    localPosition = glm::vec3(0);
    rotation = QUAT1;
    localRotation = QUAT1;
    pseudoScale = glm::vec3(1);
    localScale = glm::vec3(1);
    parent = nullptr;
    mesh = nullptr;
}

GameObject::GameObject(Mesh *mesh, glm::vec3 position, glm::vec3 scale, glm::quat rotation)
    : GameObject(nullptr, mesh, position, scale, rotation) {}

GameObject::GameObject(glm::vec3 position, glm::vec3 scale, glm::quat rotation)
    : GameObject(nullptr, nullptr, position, scale, rotation) {}

GameObject::~GameObject()
{
    if (parent != nullptr) {
        parent->DetachChild(this);
    }
    for (auto child : children) {
        child->parent = nullptr;
    }
    children.clear();
}

GameObject *GameObject::CreateChild(Mesh *mesh, glm::vec3 position, 
                                    glm::vec3 scale, glm::quat rotation) 
{
    return new GameObject(this, mesh, position, scale, rotation);
}

GameObject *GameObject::CreateChild(glm::vec3 position,
                                    glm::vec3 scale, glm::quat rotation)
{
    return new GameObject(parent, nullptr, position, scale, rotation);
}

std::unordered_set<GameObject *> &GameObject::GetChildren()
{
    return children;
}

void GameObject::AddChild(GameObject *child, bool keepWorldPosition)
{
    if (child->parent != nullptr)
        child->parent->DetachChild(child);

    children.insert(child);

    child->parent = this;
    if (keepWorldPosition) {
        // trigger a recalculation of the child's local position and rotation
        child->SetPositionDirty(child->position);
        child->SetRotationDirty(child->rotation);
        child->SetPseudoScaleDirty(child->pseudoScale);
    } else {
        // trigger a recalculation of the child's world position and rotation
        child->SetLocalPositionDirty(child->localPosition);
        child->SetLocalRotationDirty(child->localRotation);
        child->SetLocalScaleDirty(child->localScale);
    }
    RecalculateMatrix();
}

void GameObject::DetachChild(GameObject *child)
{
    children.erase(child);
    child->parent = nullptr;
}

glm::vec3 GameObject::GetLocalPosition() { return localPosition; }
glm::quat GameObject::GetLocalRotation() { return localRotation; }
glm::vec3 GameObject::GetLocalScale() { return localScale; }
glm::vec3 GameObject::GetPosition() { return position; }
glm::quat GameObject::GetRotation() { return rotation; }
glm::vec3 GameObject::GetPseudoScale() { return pseudoScale; }

void GameObject::SetLocalPositionDirty(glm::vec3 newLocalPos)
{
    localPosition = newLocalPos;
    if (parent == nullptr) {
        position = newLocalPos;
    } else {
        glm::vec3 disp = parent->pseudoScale * newLocalPos;
        disp = parent->rotation * disp;
        position = parent->position + disp;
    }

    for (auto child : children)
    {
        child->SetLocalPositionDirty(child->localPosition);
    }
}
void GameObject::SetLocalPosition(glm::vec3 newLocalPos)
{
    SetLocalPositionDirty(newLocalPos);
    RecalculateMatrix();
}

void GameObject::SetPositionDirty(glm::vec3 newPosition)
{
    position = newPosition;
    if (parent == nullptr) {
        localPosition = newPosition;
    } else {
        glm::vec3 disp = newPosition - parent->position;
        disp = glm::inverse(parent->rotation) * disp;
        localPosition = disp / parent->pseudoScale;
    }

    for (auto child : children)
    {
        child->SetLocalPositionDirty(child->localPosition);
    }
}
void GameObject::SetPosition(glm::vec3 newPosition)
{
    SetPositionDirty(newPosition);
    RecalculateMatrix();
}

void GameObject::SetLocalScaleDirty(glm::vec3 newLocalScale)
{
    localScale = newLocalScale;
    pseudoScale = (parent == nullptr) ? newLocalScale : newLocalScale * parent->pseudoScale;
    for (auto child : children)
    {
        child->SetLocalScaleDirty(child->localScale);
    }
}
void GameObject::SetLocalScale(glm::vec3 newLocalScale)
{
    SetLocalScaleDirty(newLocalScale);
    RecalculateMatrix();
}

void GameObject::SetPseudoScaleDirty(glm::vec3 newPseudoScale)
{
    pseudoScale = newPseudoScale;
    localScale = (parent == nullptr) ? newPseudoScale : newPseudoScale / parent->pseudoScale;
    for (auto child : children)
    {
        child->SetPseudoScaleDirty(child->pseudoScale);
    }
}
void GameObject::SetPseudoScale(glm::vec3 newPseudoScale)
{
    SetPseudoScaleDirty(newPseudoScale);
    RecalculateMatrix();
}

void GameObject::SetLocalRotationDirty(glm::quat newLocalRotation)
{
    localRotation = newLocalRotation;
    rotation = ((parent == nullptr) ? QUAT1 : parent->rotation) * newLocalRotation;
    forward = rotation * glm::vec3_forward;
    right = rotation * glm::vec3_right;
    up = rotation * glm::vec3_up;
    for (auto child : children)
    {
        if (child->fixedRotation)
            child->SetRotationDirty(child->rotation);
        else
            child->SetLocalRotationDirty(child->localRotation);
        child->SetLocalPositionDirty(child->localPosition);
    }
}
void GameObject::SetLocalRotation(glm::quat newLocalRotation)
{
    SetLocalRotationDirty(newLocalRotation);
    RecalculateMatrix();
}

void GameObject::SetRotationDirty(glm::quat newRotation)
{
    rotation = newRotation;
    localRotation = ((parent == nullptr) ? QUAT1 : glm::inverse(parent->rotation)) * newRotation;
    forward = rotation * glm::vec3_forward;
    right = rotation * glm::vec3_right;
    up = rotation * glm::vec3_up;
    for (auto child : children)
    {
        if (child->fixedRotation)
            child->SetRotationDirty(child->rotation);
        else
            child->SetLocalRotationDirty(child->localRotation);
        child->SetLocalPositionDirty(child->localPosition);
    }
}
void GameObject::SetRotation(glm::quat newRotation)
{
    SetRotationDirty(newRotation);
    RecalculateMatrix();
}

glm::vec3 GameObject::GetForward() { return forward; }
glm::vec3 GameObject::GetRight() { return right; }
glm::vec3 GameObject::GetUp() { return up; }

void GameObject::RecalculateMatrix()
{
    objectToWorldMatrix = (parent == nullptr ? glm::mat4(1) : parent->objectToWorldMatrix) *
        transform::Translate(localPosition) * 
        transform::Rotate(localRotation) * 
        transform::Scale(localScale);

    OnTransformChange();

    for (auto child : children)
        child->RecalculateMatrix();
}

void GameObject::Translate(glm::vec3 translation, bool local)
{
    if (local)
        SetLocalPosition(localPosition + translation);
    else
        SetPosition(position + translation);
}

void GameObject::Rotate(glm::quat rotation, bool local)
{
    if (local)
        SetLocalRotation(rotation * this->localRotation);
    else {
        SetRotation(rotation * this->rotation);
    }
}

void GameObject::Rotate(glm::vec3 eulerAngles, bool local)
{
    Rotate(glm::quat(eulerAngles), local);
}

void GameObject::Scale(glm::vec3 scale, bool local)
{
    if (local)
        SetLocalScale(localScale * scale);
    else
        SetPseudoScale(pseudoScale * scale);
}

glm::mat4 GameObject::ObjectToWorldMatrix() { return objectToWorldMatrix; }

glm::mat4 GameObject::WorldToObjectMatrix()
{
    // since this operation is not very used, we can afford to compute it on demand
    return glm::inverse(objectToWorldMatrix);
}

// added for completeness; not used
glm::vec3 GameObject::ObjectToWorldPosition(glm::vec3 point)
{
    return objectToWorldMatrix * glm::vec4(point, 1);
}

glm::vec3 GameObject::WorldToObjectPosition(glm::vec3 point)
{
    return WorldToObjectMatrix() * glm::vec4(point, 1);
}

const HitArea &GameObject::GetHitArea() { return *hitArea; }

void GameObject::SetHitArea(Shape &&shape, glm::vec3 offset, 
                            glm::vec3 scale, glm::quat rotation)
{
    GameObject *support = new GameObject(this, nullptr, offset, scale, rotation);
    hitArea = shape.CreateHitArea(support);
}

bool GameObject::Contains(glm::vec3 point)
{
    if (hitArea->support == nullptr)
        return false;
    glm::vec3 objectPoint = hitArea->support->WorldToObjectPosition(point);
    return hitArea->Contains(objectPoint);
}

// This method is strongly limited as proper collision detection requires a lot
// more research and work. The user must respect the following contract:
// - NO transformations (scale, rotation) done on the object should change the shape
//  of the hit area (e.g. no non-uniform scaling for sphere, no rotation for box)
// - This includes the parent's transformations
// - Mirroring is OK
// Failure to respect this contract will result in incorrect collision detection.
bool GameObject::Collides(GameObject *other, CollisionEventPtr &event, CollisionEventPtr &otherEvent)
{
    if (hitArea->support == nullptr || other->hitArea->support == nullptr)
        return false;

    bool collided = hitArea->Collides(other->hitArea, event, otherEvent);
    event->gameObject = other;
    otherEvent->gameObject = this;
    return collided;
}

void GameObject::SetBoxHitArea(float width, float height, float depth, glm::vec3 offset, 
                               glm::vec3 scale, glm::quat rotation)
{
    SetHitArea(BoxShape(width, height, depth), offset, scale, rotation);
}

void GameObject::SetSphereHitArea(float radius, glm::vec3 offset)
{
    SetHitArea(SphereShape(radius), offset);
    hitArea->support->fixedRotation = true;
}

GameObject *GameObject::InertDeepCopy(bool keepWorldPosition)
{
    GameObject *copy = keepWorldPosition ? 
        new GameObject(mesh, position, localScale, rotation) : 
        new GameObject(mesh, localPosition, localScale, localRotation);

    for (auto child : children)
        copy->AddChild(child->InertDeepCopy(false), false);
    return copy;
}