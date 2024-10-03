
#pragma once
#include "Graphics.h"
#include <vector>

class Sphere
{
public:
	Sphere(Graphics& gfx,
		float patternSeed = 0.f,
		DirectX::XMFLOAT3 pos = { 0,0,0 },
		DirectX::XMFLOAT3 scale = { 1,1,1 },
		DirectX::FXMVECTOR rotation = DirectX::XMQuaternionIdentity());
	void Draw(Graphics& gfx);

	void setPosition(DirectX::XMFLOAT3 newPos);
	void setScaling(DirectX::XMFLOAT3 newScaling);
	void setScaling(float factor); // wraps to above
	void setRotation(DirectX::FXMVECTOR quaternion);
	

private:
	// Const Buffer structure
	struct ConstantBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldViewProj;
		float perlinRNGSeed;
	} ConstBuffer;

	// Vertex structure
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
	};
	// Storing all these properties is pretty wasteful since we have a CB
	// embedded in the class, but it shouldn't bottleneck anything 
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scaling; // (x factor, y factor, z factor)
	DirectX::XMVECTOR rotation; // quaternion for rotation 
	static UINT indCount;
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
	// returns index count
	static UINT InitSharedResources(Graphics& gfx);

	// Update the const buffer based on internal cube params
	void updateCB();

	// Helper to generate the geometry of the sphere
	static void GenerateGeometry(size_t subdivisions, std::vector<Sphere::Vertex>& out_vertices, std::vector<unsigned short>& out_indices);

};