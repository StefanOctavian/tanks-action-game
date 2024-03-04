#include <iostream>
#include "controlledscene3d.h"
#include "transform3d.h"
#include "camera.h"
#include "material.h"
#include "assets.h"

#define CAMERA_INIT_FOVY 60
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720
#define CAMERA_INIT_ZNEAR 0.01f
#define CAMERA_INIT_ZFAR 300.0f

using namespace engine;

void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

ControlledScene3D::ControlledScene3D()
{
    gameObjects.reserve(70);
    toDestroy.reserve(10);
    // usually a scene doesn't have more than a main camera and a secondary one
    cameras.reserve(2);
    layers.assign(32, std::unordered_set<GameObject *>());
    collisionMasks.assign(32, 0);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}

ControlledScene3D::~ControlledScene3D()
{
    for (auto &gameObject : gameObjects)
        delete gameObject;

    gameObjects.clear();
    toDestroy.clear();
    cameras.clear();
}

void ControlledScene3D::RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    GLuint loc_view_matrix = glGetUniformLocation(shader->program, "WIST_VIEW_MATRIX");
    GLuint loc_projection_matrix = glGetUniformLocation(shader->program, "WIST_PROJECTION_MATRIX");
    GLuint loc_model_matrix = glGetUniformLocation(shader->program, "WIST_MODEL_MATRIX");
    GLuint loc_mvp_matrix = glGetUniformLocation(shader->program, "WIST_MVP");
    GLuint loc_eye_pos = glGetUniformLocation(shader->program, "WIST_EYE_POSITION");

    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(mainCamera->GetViewMatrix()));
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(mainCamera->GetProjectionMatrix()));
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    if (loc_mvp_matrix != -1) {
        // only compute the MVP matrix if the shader uses it
        glm::mat4 mvp = mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix() * modelMatrix;
        glUniformMatrix4fv(loc_mvp_matrix, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    if (loc_eye_pos != -1) {
        glUniform4fv(loc_eye_pos, 1, glm::value_ptr(mainCamera->GetPositionGeneralized()));
    }

    mesh->UseMaterials(false); // To whoever wrote gfxc: I hate you for this. Took me 3 days to figure out why my textures weren't working!!
    mesh->Render();
}

void ControlledScene3D::RenderMeshCustomMaterial(Mesh *mesh, Material material, const glm::mat4 &modelMatrix)
{
    if (!mesh || !material.shader || !material.shader->GetProgramID())
        return;

    material.Use();
    RenderMesh(mesh, material.shader, modelMatrix);
}

void ControlledScene3D::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ControlledScene3D::Init()
{
    std::cout << "ControlledScene3D::Init()" << std::endl;
    // ignore the fact that SimpleScene already provides a camera. In the future,
    // I plan on removing the entire framework and replacing it with my own.
    GetCameraInput()->SetActive(false);
    mainCamera = new Camera(glm::vec3(0, 1.4, 0), glm::vec3_forward, glm::vec3_up);
    mainCamera->SetPerspective(CAMERA_INIT_FOVY,
                               DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT,
                               CAMERA_INIT_ZNEAR, CAMERA_INIT_ZFAR);
    cameras.push_back(mainCamera);

    windowResolution = window->GetResolution();
    //glViewport(0, 0, windowResolution.x, windowResolution.y);
    ResizeDrawArea();

    Assets::lookupDirectory = PATH_JOIN(window->props.selfDir, SOURCE_PATH::MAIN, "wisteria_engine", "shaders");
    Assets::AddPath("Default.VS", "Default.VS.glsl");
    Assets::AddPath("Default.VertexColor.FS", "Default.VertexColor.FS.glsl");
    Assets::AddPath("PlainColor.FS", "PlainColor.FS.glsl");
    Assets::AddPath("Default.Texture.FS", "Default.Texture.FS.glsl");
    Assets::AddPath("Transform.Texture.VS", "Transform.Texture.VS.glsl");

    Assets::LoadShader("VertexColor", "Default.VS", "Default.VertexColor.FS");
    Assets::LoadShader("PlainColor", "Default.VS", "PlainColor.FS");
    Assets::LoadShader("Texture", "Default.VS", "Default.Texture.FS");
    Assets::LoadShader("TransformTexture", "Transform.Texture.VS", "Default.Texture.FS");
    Assets::lookupDirectory = window->props.selfDir;
    this->Initialize();
}

void ControlledScene3D::AddToScene(GameObject *gameObject)
{
    gameObjects.insert(gameObject);
    gameObject->scene = this;
    for (auto &child : gameObject->GetChildren()) {
        AddToScene(child);
    }
}

void ControlledScene3D::Destroy(GameObject *gameObject)
{
    toDestroy.insert(gameObject);
    for (auto &child : gameObject->GetChildren()) {
        Destroy(child);
    }
}

void ControlledScene3D::AddToLayer(GameObject *gameObject, int layer)
{
    if (layer > 32) {
        std::cerr << "Wisteria Engine only supports 32 layers.\n";
        exit(1);
    }
    layers[layer].insert(gameObject);
}

void ControlledScene3D::RemoveFromLayer(GameObject *gameObject, int layer)
{
    if (layer > 32) {
        std::cerr << "Wisteria Engine only supports 32 layers.\n";
        exit(1);
    }
    layers[layer].erase(gameObject);
}

struct pair_hash {
    inline std::size_t operator()(const std::pair<GameObject *, GameObject *> &v) const {
        return reinterpret_cast<uintptr_t>(v.first) ^ reinterpret_cast<uintptr_t>(v.second);
    }
};

void ControlledScene3D::Update(float deltaTimeSeconds)
{
    deltaTime = deltaTimeSeconds * timeScale;
    unscaledDeltaTime = deltaTimeSeconds;

    for (auto &camera : cameras) {
        // if (!camera->active)
        //     continue;
        mainCamera = camera;
        // std::cout << "drawArea: (" << drawAreaX << ", " << drawAreaY << ", " << drawAreaWidth << ", " << drawAreaHeight << ")\n";
        // std::cout << "viewport: (" << (int)(mainCamera->viewportX * drawAreaWidth) << ", " << (int)(mainCamera->viewportY * drawAreaHeight) << ", " << (int)(mainCamera->viewportWidth * drawAreaWidth) << ", " << (int)(mainCamera->viewportHeight * drawAreaHeight) << ")\n";
        glViewport(drawAreaX + (int)(mainCamera->viewportX * drawAreaWidth), 
                   drawAreaY + (int)(mainCamera->viewportY * drawAreaHeight),
                   (int)(mainCamera->viewportWidth * drawAreaWidth), 
                   (int)(mainCamera->viewportHeight * drawAreaHeight));
        for (auto gameObject : gameObjects) {
            DrawGameObject(gameObject);
        }
    }
    mainCamera = cameras[0];

    Tick();

    CheckCollisions();
    
    for (auto gameObject : toDestroy) {
        if (gameObjects.find(gameObject) == gameObjects.end())
            continue;

        gameObjects.erase(gameObject);
        for (int layer = 0; layer < 32; ++layer)
            RemoveFromLayer(gameObject, layer);

        delete gameObject;
    }
    toDestroy.clear();
}

void ControlledScene3D::CheckCollisions()
{
    std::unordered_set<std::pair<GameObject *, GameObject *>, pair_hash> pairs;
    for (int layer1 = 0; layer1 < 32; ++layer1) {
        for (int layer2 = layer1; layer2 < 32; ++layer2) {
            if ((collisionMasks[layer1] & (1 << layer2)) == 0)
                continue;
            
            for (auto gameObject1 : layers[layer1]) {
                for (auto gameObject2 : layers[layer2]) {
                    if (gameObject1 == gameObject2 ||
                        pairs.find({gameObject1, gameObject2}) != pairs.end()) continue;
                    
                    std::unique_ptr<CollisionEvent> event1, event2;
                    pairs.insert(std::make_pair(gameObject1, gameObject2));
                    if (gameObject1->Collides(gameObject2, event1, event2)) {
                        event1->Dispatch(gameObject1);
                        event2->Dispatch(gameObject2);
                    }
                }
            }
        }
    }
}

void ControlledScene3D::DrawGameObject(GameObject *gameObject)
{
    if (gameObject->mesh) {
        glm::mat4 modelMatrix = gameObject->ObjectToWorldMatrix();
        if (gameObject->material.shader) {
            RenderMeshCustomMaterial(gameObject->mesh, gameObject->material, modelMatrix);
        } else {
            RenderMesh(gameObject->mesh, Assets::shaders["VertexColor"], modelMatrix);
        }
    }

    float objectDeltaTime = gameObject->useUnscaledTime ? unscaledDeltaTime : deltaTime;
    if (gameObject->acceleration != glm::vec3(0)) {
        gameObject->velocity += gameObject->acceleration * objectDeltaTime;
    }
    if (gameObject->velocity != glm::vec3(0)) {
        glm::vec3 position = gameObject->GetLocalPosition();
        position += gameObject->velocity * objectDeltaTime;
        gameObject->SetLocalPosition(position);
    }
    if (gameObject->angularVelocity != glm::vec3(0)) {
        glm::quat rotation = gameObject->GetLocalRotation();
        float angularSpeed = glm::length(gameObject->angularVelocity);
        glm::vec3 axis = gameObject->angularVelocity / angularSpeed;
        rotation = glm::rotate(rotation, angularSpeed * objectDeltaTime, axis);
        gameObject->SetLocalRotation(rotation);
    }

    for (auto &child : gameObject->GetChildren()) {
        DrawGameObject(child);
    }
}

void ControlledScene3D::OnInputUpdate(float deltaTime, int mods)
{
    this->OnInputUpdate(mods);
}

void ControlledScene3D::ResizeDrawArea()
{
    // windowResolution = window->GetResolution();
    // mainCamera->SetPerspective(60, 1280 / 720.f, 0.01f, 200.0f);

    const float aspectRatio = 1280 / 720.0f;  // TODO(me): Remove hardcoded values
    const float windowAspectRatio = windowResolution.x / (float)windowResolution.y;
    if (windowAspectRatio > aspectRatio) {
        const int w = (int)(windowResolution.y * aspectRatio);
        //glViewport((windowResolution.x - w) / 2, 0, w, windowResolution.y);
        drawAreaX = (windowResolution.x - w) / 2;
        drawAreaY = 0;
        drawAreaWidth = w;
        drawAreaHeight = windowResolution.y;
    }
    else {
        const int h = (int)(windowResolution.x / aspectRatio);
        // glViewport(0, (windowResolution.y - h) / 2, windowResolution.x, h);
        drawAreaX = 0;
        drawAreaY = (windowResolution.y - h) / 2;
        drawAreaWidth = windowResolution.x;
        drawAreaHeight = h;
    }
}

void ControlledScene3D::OnWindowResize(int width, int height)
{
    windowResolution = window->GetResolution();
    ResizeDrawArea();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OnResizeWindow();
}
