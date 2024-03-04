#pragma once
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "material.h"
#include "hitarea3d.h"

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

#define QUAT1 glm::quat(1, 0, 0, 0)

namespace engine
{
    class GameObject
    {
    public:
        GameObject();
        GameObject(Mesh *mesh, glm::vec3 position, glm::vec3 scale = glm::vec3(1),
                   glm::quat rotation = QUAT1);
        GameObject(glm::vec3 position, glm::vec3 scale = glm::vec3(1),
                   glm::quat rotation = QUAT1);
        virtual ~GameObject();

        GameObject *CreateChild(Mesh *mesh, glm::vec3 position, 
                                glm::vec3 scale = glm::vec3(1), glm::quat rotation = QUAT1);
        GameObject *CreateChild(glm::vec3 position,
                                glm::vec3 scale = glm::vec3(1), glm::quat rotation = QUAT1);

        // events
        virtual void OnCollision(const CollisionEvent &collision) {};
        virtual void OnCollision(const SphereBoxCollisionEvent &collision) {};
        virtual void OnCollision(const SphereSphereCollisionEvent &collision) {};
        virtual void OnTransformChange() {};

        // children
        std::unordered_set<GameObject *> &GetChildren();
        void AddChild(GameObject *child, bool keepWorldPosition = true);
        void DetachChild(GameObject *child);

        // position, scale, rotation
        glm::vec3 GetPosition();
        void SetPosition(glm::vec3 position);
        glm::quat GetRotation();
        void SetRotation(glm::quat rotation);
        glm::vec3 GetPseudoScale();
        void SetPseudoScale(glm::vec3 scale);

        glm::vec3 GetLocalPosition();
        void SetLocalPosition(glm::vec3 position);
        glm::vec3 GetLocalScale();
        void SetLocalScale(glm::vec3 scale);
        glm::quat GetLocalRotation();
        void SetLocalRotation(glm::quat rotation);

        void Translate(glm::vec3 translation, bool local = false);
        void Rotate(glm::quat rotation, bool local = false);
        void Rotate(glm::vec3 eulerAngles, bool local = false);
        void Scale(glm::vec3 scale, bool local = false);

        glm::vec3 GetForward();
        glm::vec3 GetRight();
        glm::vec3 GetUp();

        // world transformations
        glm::mat4 ObjectToWorldMatrix();
        glm::mat4 WorldToObjectMatrix();
        glm::vec3 ObjectToWorldPosition(glm::vec3 point);
        glm::vec3 WorldToObjectPosition(glm::vec3 point);

        // hit area
        HitArea const &GetHitArea();
        void SetHitArea(Shape &&shape, glm::vec3 offset = glm::vec3(0),
                        glm::vec3 scale = glm::vec3(1), glm::quat rotation = QUAT1);
        bool Contains(glm::vec3 point);
        bool Collides(GameObject *other, CollisionEventPtr &event, CollisionEventPtr &otherEvent);
        void SetBoxHitArea(float width, float height, float depth, 
                           glm::vec3 offset = glm::vec3(0), 
                           glm::vec3 scale = glm::vec3(1), glm::quat rotation = QUAT1);
        void SetSphereHitArea(float radius, glm::vec3 offset = glm::vec3(0));

        GameObject *InertDeepCopy(bool keepWorldPosition = true);

        std::string name = "";
        std::string tag = "";
        Mesh *mesh = nullptr;
        Material material;
        ControlledScene3D *scene = nullptr;

        glm::vec3 velocity = glm::vec3(0);
        glm::vec3 angularVelocity = glm::vec3(0);
        glm::vec3 acceleration = glm::vec3(0);
        bool useUnscaledTime = false;

        // if true, this gameobject will not change its world rotation when
        // its parent gameobject is transformed
        bool fixedRotation = false;

    protected:
        GameObject(GameObject *parent, Mesh *mesh, glm::vec3 position, 
                   glm::vec3 scale = glm::vec3(1), glm::quat rotation = QUAT1);

        void SetLocalPositionDirty(glm::vec3 position);
        void SetLocalRotationDirty(glm::quat rotation);
        void SetLocalScaleDirty(glm::vec3 scale);
        void SetPositionDirty(glm::vec3 position);
        void SetRotationDirty(glm::quat rotation);
        void SetPseudoScaleDirty(glm::vec3 scale);
        void RecalculateMatrix();

        glm::vec3 localPosition = glm::vec3(0);
        glm::vec3 localScale = glm::vec3(1);
        glm::quat localRotation = QUAT1;
        glm::vec3 position = glm::vec3(0);
        glm::quat rotation = QUAT1;
        glm::vec3 pseudoScale = glm::vec3(1);
        glm::vec3 forward = glm::vec3_forward;
        glm::vec3 right = glm::vec3_right;
        glm::vec3 up = glm::vec3_up;
        glm::mat4 objectToWorldMatrix = glm::mat4(1);

        GameObject *parent = nullptr;
        HitArea *hitArea;
        std::unordered_set<GameObject *> children;
    };
}