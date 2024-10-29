#include "Planet.h"
#include <cassert>

Planet::Planet(Graphics& gfx, float patternseed, DirectX::XMFLOAT3 pos /*= { 0,0,0 }*/, float radius /*= 1.0f*/)
	: Sphere(gfx, patternseed, pos, {radius, radius, radius})
	, radius(radius)
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

float Planet::getRadius() const
{
	return radius;
}

DirectX::CXMVECTOR Planet::GetVecVelocity() const
{
	return _vel;
}

void Planet::SetVecVelocity(DirectX::CXMVECTOR newVelocity)
{
	_vel = newVelocity;
}

DirectX::XMFLOAT3 Planet::GetVelocity() const
{
	DirectX::XMFLOAT3 f3vel;
	DirectX::XMStoreFloat3(&f3vel, GetVecVelocity());
	return f3vel;
}

void Planet::SetVelocity(const DirectX::XMFLOAT3& newVel)
{
	SetVecVelocity(DirectX::XMLoadFloat3(&newVel));
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

bool Planet::isRayIntersecting(DirectX::XMVECTOR rayDir, DirectX::CXMVECTOR rayOrigin)
{
	using namespace DirectX;
	
	XMVECTOR m = rayOrigin - GetVecPosition();
	float b = XMVectorGetX(XMVector3Dot(m, rayDir));
	float c = XMVectorGetX(XMVector3Dot(m, m)) - radius * radius;

	// Exit if ray's origin is outside the sphere and ray is pointing away
	if (c > 0.0f && b > 0.0f) return false;

	float discriminant = b * b - c;

	// No intersection
	if (discriminant < 0.0f) return false;

	// Ray intersects sphere
	return true;
}
