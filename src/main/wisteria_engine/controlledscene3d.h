#pragma once
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "gameobject3d.h"
#include "camera.h"
#include "meshplusplus.h"

#include "components/simple_scene.h"

namespace engine
{
    class ControlledScene3D : public gfxc::SimpleScene
    {
    public:
        ControlledScene3D();
        ~ControlledScene3D();
        void Init() override;

        void AddToScene(GameObject *gameObject);
        void Destroy(GameObject *gameObject);
        void AddToLayer(GameObject *gameObject, int layer);
        void RemoveFromLayer(GameObject *gameObject, int layer);

    protected:
        virtual void Initialize() {}; 
        virtual void Tick() {};
        virtual void OnResizeWindow() {};
        virtual void OnInputUpdate(int mods) {};

        // glm::vec2 ScreenCoordsToLogicCoords(int screenX, int screenY);

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void CheckCollisions();
        void OnInputUpdate(float deltaTime, int mods) override;
        // void FrameEnd() override;

        void ResizeDrawArea();
        void OnWindowResize(int width, int height) override;
        
        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix);
        void RenderMeshCustomMaterial(Mesh *mesh, Material material, const glm::mat4 &modelMatrix);
        void DrawGameObject(GameObject *gameObject);

    protected:
        glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);
        std::unordered_set<GameObject *> gameObjects;
        float deltaTime;
        float unscaledDeltaTime;
        float timeScale = 1;
        glm::ivec2 windowResolution;

        std::vector<Camera *> cameras;
        Camera *mainCamera;
        int drawAreaX, drawAreaY, drawAreaWidth, drawAreaHeight;

        std::vector<int> collisionMasks;

    private:
        std::unordered_set<GameObject *> toDestroy;
        std::vector<std::unordered_set<GameObject *>> layers;
    };
} // namespace engine