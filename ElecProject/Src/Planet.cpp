#include "Planet.h"
#include <cassert>

Planet::Planet(Graphics& gfx, float patternseed, DirectX::XMFLOAT3 pos /*= { 0,0,0 }*/, float radius /*= 1.0f*/)
	: Sphere(gfx, patternseed, pos, {radius/2, radius/2, radius/2})
{

}

// Returns mass in KG
float Planet::GetMass() const
{
	return _mass;
}

void Planet::SetMass(float newMass)
{
	assert(newMass > 0);
	_mass = newMass;
	_invMass = 1.0f / _mass;
}

DirectX::CXMVECTOR Planet::GetVelocity() const
{
	return _vel;
}

void Planet::SetVelocity(DirectX::CXMVECTOR newVelocity)
{
	_vel = newVelocity;
}

DirectX::XMVECTOR Planet::calcAcceleration(DirectX::CXMVECTOR force) const
{
	return DirectX::XMVectorScale(force, _invMass);
}

DirectX::XMVECTOR Planet::GetVecPosition() const
{
	auto p = GetPosition();
	return DirectX::XMLoadFloat3(&p);
}

void Planet::SetVecPosition(DirectX::CXMVECTOR newPos)
{
	DirectX::XMFLOAT3 p;
	DirectX::XMStoreFloat3(&p, newPos);
	SetPosition(p);
}
