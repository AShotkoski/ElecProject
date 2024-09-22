#include "Camera.h"

namespace dx = DirectX;

Camera::Camera()
	:
	view(dx::XMMatrixLookAtLH(
		dx::XMVectorSet(0.f, 0.f, -5.f, 1.f), // eye pos
		dx::XMVectorSet(0.f, 0.f, 0.f, 1.f), // focal pt
		dx::XMVectorSet(0.f, 1.f, 0.f, 0.f))) // up direction)
{
}

const DirectX::XMMATRIX& Camera::GetViewMatrix() const
{
	return view;
}

