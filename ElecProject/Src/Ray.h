#pragma once
#include <DirectXMath.h>

struct Ray
{
	DirectX::XMVECTOR origin;
	DirectX::XMVECTOR direction;
};

class RayUtils
{
public:
	// Create a ray in world space with the origin at the camera in the direction
	// determined from the NDC coords of x and y
	static Ray fromNDC(float x, float y, DirectX::CXMMATRIX invView, DirectX::CXMMATRIX invProj)
	{
		using namespace DirectX;

		// Define ndc points at near and far planes
		XMVECTOR nearPt = XMVectorSet(x, y, 0.f, 1.f);
		XMVECTOR farPt = XMVectorSet(x, y, 1.f, 1.f);

		// Unproject ndc points into view space
		nearPt = XMVector3TransformCoord(nearPt, invProj);
		farPt = XMVector3TransformCoord(farPt, invProj);

		// Transform from view to world
		nearPt = XMVector3TransformCoord(nearPt, invView);
		farPt = XMVector3TransformCoord(farPt, invView);

		// Get the origin
		Ray ret;
		ret.origin = XMVectorSetW(XMVectorZero(), 1.0f);
		ret.origin = XMVector3TransformCoord(ret.origin, invView);

		// Get the direction
		ret.direction = XMVector3Normalize(farPt - ret.origin);

		return ret;
	}

};