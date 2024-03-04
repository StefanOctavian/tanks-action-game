#pragma once
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "utils/glm_utils.h"

namespace engine
{
    struct HitArea;
    struct BoxHitArea;
    struct SphereHitArea;
    class CollisionEvent;
    class SphereBoxCollisionEvent;
    class GameObject;

    typedef std::unique_ptr<CollisionEvent> CollisionEventPtr;

    // a shape can be anything, but it must be able to create a hitarea
    struct Shape {
        virtual ~Shape() = default;
        virtual HitArea *CreateHitArea(GameObject *support) = 0;
    };

    // the following uses double dispatch with a double dispatch table and no C++ RTTI
    // (typeid(expression) and dynamic_cast<type>(expression)). It uses typeid(type);
    // The visitor pattern was not suitable here because I don't want the hierarchy to
    // be closed, that is, the base class has to know about all the derived classes,
    // which is not extensible. 

    // a hit area has a support gameobject and determines collisions
    // note that the hitarea does not contain the shape, as it carries no information
    // instead, concrete hitareas contain concrete shapes
    struct HitArea {
        HitArea(GameObject *support) : support(support) {}
        virtual ~HitArea() = default;

        GameObject *support;

        virtual bool Contains(glm::vec3 point) = 0;
        bool Collides(HitArea *other, CollisionEventPtr &event, CollisionEventPtr &otherEvent);

    protected:
        typedef bool (*CollidesFunc)(HitArea *, HitArea *, CollisionEventPtr &, CollisionEventPtr &);
        virtual std::type_index GetType() = 0;
        typedef std::pair<std::type_index, std::type_index> type_pair;
        struct pair_hash {
            std::size_t operator() (const type_pair &pair) const {
                return pair.first.hash_code() ^ pair.second.hash_code();
            }
        };
        static std::unordered_map<type_pair, CollidesFunc, pair_hash> collisionFuncs;
        // this is called by the derived classes to register their collision functions;
        // when extending the engine with new hitareas, this function must be called
        // for each new pair of hitareas that can collide. The func parameter must be
        // cast to CollidesFunc, even though this cast is unsafe in general. The user
        // must ensure that the passed function takes two HitArea pointers of the types
        // specified by the type_index parameters.
        static void RegisterCollisionFunc(std::type_index type1, std::type_index type2, 
                                          CollidesFunc func);
    };

    struct BoxShape : public Shape {
        BoxShape() = default;
        BoxShape(float width, float height, float depth) 
            : width(width), height(height), depth(depth) {};
        HitArea *CreateHitArea(GameObject *support) override;

        float width, height, depth;
    };

    struct BoxHitArea : public HitArea
    {
        BoxHitArea(GameObject *support, BoxShape shape) 
            : HitArea(support), shape(shape) {}
        BoxShape shape;
        bool Contains(glm::vec3 point) override;

    protected:
        std::type_index GetType() override { return typeid(BoxHitArea); }

    private:
        static const struct init { init(); } initializer;
    };

    bool CollidesBoxBox(BoxHitArea *, BoxHitArea *, CollisionEventPtr &, CollisionEventPtr &);
    bool CollidesBoxSphere(BoxHitArea *, SphereHitArea *, CollisionEventPtr &, CollisionEventPtr &);
    bool CollidesSphereBox(SphereHitArea *, BoxHitArea *, CollisionEventPtr &, CollisionEventPtr &);

    struct SphereShape : public Shape {
        SphereShape() = default;
        SphereShape(float radius) : radius(radius) {};
        HitArea *CreateHitArea(GameObject *support) override;
        float radius;
    };

    struct SphereHitArea : public HitArea
    {
        SphereHitArea(GameObject *support, SphereShape shape) 
            : HitArea(support), shape(shape) {}
        SphereShape shape;
        bool Contains(glm::vec3 point) override;

    protected:
        std::type_index GetType() override { return typeid(SphereHitArea); }

    private:
        static const struct init { init(); } initializer;
    };

    bool CollidesSphereSphere(SphereHitArea *, SphereHitArea *, CollisionEventPtr &, CollisionEventPtr &);

    class CollisionEvent
    {
        friend class ControlledScene3D;  // only the scene can dispatch collision events
    public:
        CollisionEvent(GameObject *gameObject): gameObject(gameObject) {}
        virtual ~CollisionEvent() = default;
        GameObject *gameObject;
    private:
        virtual void Dispatch(GameObject *target);
    };

    class SphereBoxCollisionEvent : public CollisionEvent
    {
    public:
        SphereBoxCollisionEvent(GameObject *gameObject, glm::vec3 closestPoint, 
                                glm::vec3 displacement, float distance)
            : CollisionEvent(gameObject), closestPoint(closestPoint), 
              displacement(displacement), distance(distance) {}
        glm::vec3 closestPoint;
        glm::vec3 displacement;
        float distance;

    private:
        void Dispatch(GameObject *target) override;
    };

    class SphereSphereCollisionEvent : public CollisionEvent
    {
    public:
        SphereSphereCollisionEvent(GameObject *gameObject, glm::vec3 displacement, 
                                   float distance, float sumRadius)
            : CollisionEvent(gameObject), displacement(displacement), 
              distance(distance), sumRadius(sumRadius) {}
        glm::vec3 displacement;
        float distance, sumRadius;
    
    private:
        void Dispatch(GameObject *target) override;
    };
}

// IF YOU MOVE THIS INCLUDE TO THE TOP OF THE FILE, ALL HELL BREAKS LOOSE
// This is because of the circular dependency between hitarea.h and gameobject.h
#include "gameobject3d.h"