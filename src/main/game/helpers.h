#pragma once
#include <iostream>

// layers
#define LAYER_TANKS 0
#define LAYER_BUILDINGS 1
#define LAYER_CANNONBALLS 2

namespace game
{
    inline float randomFloat(float min, float max)
    {
        return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
    }

    inline int randomInt(int min, int max)
    {
        return min + (rand() % (max - min + 1));
    }
}