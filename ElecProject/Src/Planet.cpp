#include "Planet.h"
#include "Ray.h"
#include "ImGuiCustom.h"
#include <cassert>

Planet::Planet(Graphics& gfx, float patternseed, DirectX::XMFLOAT3 pos /*= { 0,0,0 }*/, float radius /*= 1.0f*/)
	: Sphere(gfx, patternseed, pos, {radius, radius, radius})
	, radius(radius)
{

}

void Planet::Draw(Graphics& gfx)
{
	Sphere::Draw(gfx);
	if (ControlWindowEnabled)
		DrawControlWindow();
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

bool Planet::isRayIntersecting(const Ray& ray) const
{
	using namespace DirectX;
	
	XMVECTOR m = ray.origin - GetVecPosition();
	float b = XMVectorGetX(XMVector3Dot(m, ray.direction));
	float c = XMVectorGetX(XMVector3Dot(m, m)) - radius * radius;

	// Exit if ray's origin is outside the sphere and ray is pointing away
	if (c > 0.0f && b > 0.0f) return false;

	float discriminant = b * b - c;

	// No intersection
	if (discriminant < 0.0f) return false;

	// Ray intersects sphere
	return true;
}

void Planet::EnableControlWindow()
{
	ControlWindowEnabled = true;
}

void Planet::DrawControlWindow()
{
	bool outdatedProperties = false;
	
	std::string title = "Planet";
	title.append("##");
	title.append(std::to_string(reinterpret_cast<uintptr_t>(this)));

	ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Position
	outdatedProperties |= ImGui::DragFloat3("Position", &position.x, 0.25f);
	// Mass
	ImGui::InputFloat("Mass", &_mass, 0, 0, "%e");
	// Velocity
	auto vel = GetVelocity();
	if (ImGui::DragFloat3("Velocity", &vel.x, 0.25f))
	{
		SetVelocity(vel);
	}
	ImGui::End();

	if (outdatedProperties)
		Sphere::updateCB();
}

void Planet::DisableControlWindow()
{
	ControlWindowEnabled = false;
}

bool Planet::isControlWindowEnabled() const
{
	return ControlWindowEnabled;
}

void Planet::ToggleControlWindow()
{
	ControlWindowEnabled = !ControlWindowEnabled;
}
