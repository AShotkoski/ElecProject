#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	const DirectX::XMMATRIX& GetViewMatrix() const;
private:
	DirectX::XMMATRIX view;
};
