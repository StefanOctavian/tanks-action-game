#pragma once
#include <iostream>
#include "gameobject3d.h"

namespace engine
{
    class Camera : public GameObject
    {
    public:
        // forward and up need to be normalized
        Camera(glm::vec3 pos, glm::vec3 forward, glm::vec3 up)
            : GameObject(pos, glm::vec3(1), glm::quatLookAt(-forward, up))
        // quatLookAt's default looking direction is -Z but gameobjects look at +Z
        {
            viewMatrix = glm::lookAt(position, position + this->forward, this->up);
            projectionMatrix = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
        }

        void SetPerspective(float fovy, float aspect, float zNear, float zFar)
        {
            projectionMatrix = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
        }

        void SetOrthographic(float left, float right, float bottom, float top, float near, float far)
        {
            projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
        }

        glm::mat4 GetViewMatrix() const
        {
            return viewMatrix;
        }

        glm::mat4 GetProjectionMatrix() const
        {
            return projectionMatrix;
        }

        void RotateAround(float distance, float angle, glm::vec3 localAxis)
        {
            Translate(distance * forward);
            Rotate(glm::angleAxis(angle, localAxis));
            Translate(-distance * forward);
        }

        void RotateAround(float distance, float angle)
        {
            RotateAround(distance, angle, up);
        }

        void OnTransformChange() override
        {
            viewMatrix = glm::lookAt(position, position + forward, up);
        }

        bool IsOrthographic() const
        {
            return isOrthographic;
        }

        bool IsPerspective() const
        {
            return !isOrthographic;
        }

        // these numbers are relative to the window size and the set aspect ratio
        // width = 0.5 means half of the drawing area width
        // x = 0, y = 0 means the bottom left corner of the drawing area
        // the drawing area has set aspect ratio in ControlledScene 
        void SetViewport(float x, float y, float width, float height)
        {
            viewportX = x;
            viewportY = y;
            viewportWidth = width;
            viewportHeight = height;
        }

        glm::vec4 GetPositionGeneralized() const
        {
            return glm::vec4(isOrthographic ? forward : position, !isOrthographic);
        }

        float viewportX = 0;
        float viewportY = 0;
        float viewportWidth = 1;
        float viewportHeight = 1;
        bool active = true;

    private:
        bool isOrthographic = false;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };
}