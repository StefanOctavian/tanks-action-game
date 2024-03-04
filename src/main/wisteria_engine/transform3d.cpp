#include "transform3d.h"

using namespace engine;

glm::mat4 transform::Translate(float translateX, float translateY, float translateZ)
{
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        translateX, translateY, translateZ, 1
    );
}

glm::mat4 transform::Translate(glm::vec3 translation)
{
    return Translate(translation.x, translation.y, translation.z);
}

glm::mat4 transform::Scale(float scaleX, float scaleY, float scaleZ)
{
    return glm::mat4(
        scaleX, 0, 0, 0,
        0, scaleY, 0, 0,
        0, 0, scaleZ, 0,
        0, 0, 0, 1
    );
}

glm::mat4 transform::Scale(glm::vec3 scale)
{
    return Scale(scale.x, scale.y, scale.z);
}

glm::mat4 transform::Rotate(glm::quat rotation)
{
    return glm::mat4_cast(rotation);
}

glm::mat4 transform::RotateOX(float radians)
{
    return transform::Rotate(glm::angleAxis(radians, glm::vec3(1, 0, 0)));
}

glm::mat4 transform::RotateOY(float radians)
{
    return transform::Rotate(glm::angleAxis(radians, glm::vec3(0, 1, 0)));
}

glm::mat4 transform::RotateOZ(float radians)
{
    return transform::Rotate(glm::angleAxis(radians, glm::vec3(0, 0, 1)));
}


