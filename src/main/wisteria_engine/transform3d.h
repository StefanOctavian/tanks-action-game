#pragma once

#include "utils/glm_utils.h"

namespace engine::transform
{
    glm::mat4 Translate(float translateX, float translateY, float translateZ);
    glm::mat4 Translate(glm::vec3 translation);
    glm::mat4 Scale(float scaleX, float scaleY, float scaleZ);
    glm::mat4 Scale(glm::vec3 scale);
    glm::mat4 Rotate(glm::quat rotation);
    glm::mat4 RotateOX(float radians);
    glm::mat4 RotateOY(float radians);
    glm::mat4 RotateOZ(float radians);
}