//
// The planet class is sphere that has additional properties 
// these include:
// - Construction from radius
// - Physics properties (mass, vel, ...)
//

#pragma once
#include "Sphere.h"

class Planet :
    public Sphere
{
public:
    // patternseed is a small float value that makes the random terrain unique to this planet
    Planet(Graphics& gfx,
        float patternseed,
        DirectX::XMFLOAT3 pos = { 0,0,0 },
        float radius = 1.0f);

};

