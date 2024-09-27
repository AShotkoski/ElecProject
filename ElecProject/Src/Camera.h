#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	DirectX::XMMATRIX GetMatrix() const noexcept;
	DirectX::XMMATRIX GetInvMatrix() const noexcept;
	void Reset() noexcept;
	void UpdateView(DirectX::XMFLOAT2 dView);
	void UpdatePosition(DirectX::XMFLOAT3 dPos, float dt);
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetDirectionVector() const;
	void EnableMouseControl();
	void DisableMouseControl();
	bool isMouseControlEnabled() const;
	void UpdateMovementSpeed(float factor);
private:
	void CalculateMatrices();
private:
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX invView = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT3 Position;
	bool isMouseControl = false;
	float pitch;
	float yaw;
	static constexpr float Sensitivity = 17.f / 10000.f;
	float MoveSpeed = 15.f;

	// Defaults
	static constexpr DirectX::XMFLOAT3 def_pos = { 0,0,-5.5f };
	static constexpr float def_pitch = 0.f;
	static constexpr float def_yaw = 0.f;

};

