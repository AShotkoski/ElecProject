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

    // Physics Getters and Setters
    float GetMass() const;
    void SetMass(float newMass);
    DirectX::CXMVECTOR GetVecVelocity() const;
    void SetVecVelocity(DirectX::CXMVECTOR newVelocity);
    DirectX::XMFLOAT3 GetVelocity() const;
    void SetVelocity(const DirectX::XMFLOAT3& newVel);
    // Returns the acceleration from a given force
    DirectX::XMVECTOR calcAcceleration(DirectX::CXMVECTOR force) const;
    DirectX::XMVECTOR GetVecPosition() const;
    void SetVecPosition(DirectX::CXMVECTOR newPos);
    bool isRayIntersecting(DirectX::CXMVECTOR rayDir, DirectX::CXMVECTOR rayOrigin);
private:
    // Physics attributes
    // units are all SI
    float _mass = 100.f; 
    float _invMass = 1.f / _mass;
    DirectX::XMVECTOR _vel = DirectX::XMVectorZero();
    const float radius;

};

