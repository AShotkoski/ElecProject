//
// TODO:
// 1) Refactor the const buffer to not suck
// 3) 
//
//
//

#pragma once
#include "Graphics.h"

class Cube
{
public:
	Cube(Graphics& gfx);
	void Draw(Graphics& gfx, float dt);

private:
	// Instance specific resource
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstBuffer;

	// Shared resources (shared between cube classes)
	static Microsoft::WRL::ComPtr<ID3D11VertexShader> s_pVertexShader;
	static Microsoft::WRL::ComPtr<ID3D11PixelShader> s_pPixelShader;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> s_pInputLayout;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> s_pVertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> s_pIndexBuffer;
	// Flag to track if the shared resources are initialized
	static bool s_sharedResourcesInitialized;

	// Helper function to initialize all the shared resources
	static void InitSharedResources(Graphics& gfx);


	// Const Buffer structure
	struct ConstBuffer
	{
		DirectX::XMMATRIX worldViewProj;
	};

	// Vertex structure
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
};