#include "Cube.h"
#include "Macros.h"
namespace shaders
{
#include "PixelShader.shaderheader" // If there is an error, ignore it. File gets created on compilation
#include "VertexShader.shaderheader"
}
// Namespace definitions in the implementation file
using namespace Microsoft::WRL;
namespace dx = DirectX;

// Initialize the static members globally
ComPtr<ID3D11VertexShader> Cube::s_pVertexShader;
ComPtr<ID3D11PixelShader> Cube::s_pPixelShader;
ComPtr<ID3D11InputLayout> Cube::s_pInputLayout;
ComPtr<ID3D11Buffer> Cube::s_pVertexBuffer;
ComPtr<ID3D11Buffer> Cube::s_pIndexBuffer;
bool Cube::s_sharedResourcesInitialized = false;


Cube::Cube(Graphics& gfx)
{
	HRESULT hr;
	// Initialize the shared resources if they are not yet initialized 
	if (!s_sharedResourcesInitialized)
	{
		InitSharedResources(gfx);
		s_sharedResourcesInitialized = true;
	}

	// Create the instance specific const buffer (using the cd3d11 helpers here since 
	// I remembered they existed and they make the code less verbose)
	D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(sizeof(ConstBuffer),D3D11_BIND_CONSTANT_BUFFER);
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&cbDesc, nullptr, &pConstBuffer));
}

void Cube::Draw(Graphics& gfx, float dt)
{
	dx::XMMATRIX view = dx::XMMatrixLookAtLH(
		dx::XMVectorSet(0.f, 0.f, -5.f, 1.f), // eye pos
		dx::XMVectorSet(0.f, 0.f, 0.f, 1.f), // focal pt
		dx::XMVectorSet(0.f, 1.f, 0.f, 0.f)); // up direction
	dx::XMMATRIX proj = dx::XMMatrixPerspectiveFovLH(
		dx::XM_PIDIV2, // FOV
		gfx.GetWidth() / gfx.GetHeight(), // Aspect ratio
		0.1f, // near clipping
		100.f); // far clipping
	static float theta = dt;
	dx::XMMATRIX world = dx::XMMatrixRotationRollPitchYaw(0, theta, 0.01f);
	theta += dt;

	dx::XMMATRIX worldviewproj = world * view * proj;

	worldviewproj = dx::XMMatrixTranspose(worldviewproj); // HLSL wants column major

	ConstBuffer cb;
	cb.worldViewProj = worldviewproj;

	gfx.pGetContext()->UpdateSubresource(pConstBuffer.Get(), 0, nullptr, &cb, 0, 0);

	// Set everything

	gfx.pGetContext()->VSSetConstantBuffers(0, 1, pConstBuffer.GetAddressOf());
	gfx.pGetContext()->IASetInputLayout(s_pInputLayout.Get());
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	gfx.pGetContext()->IASetVertexBuffers(0, 1, s_pVertexBuffer.GetAddressOf(), &stride, &offset);
	gfx.pGetContext()->IASetIndexBuffer(s_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	gfx.pGetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.pGetContext()->VSSetShader(s_pVertexShader.Get(), nullptr, 0);
	gfx.pGetContext()->PSSetShader(s_pPixelShader.Get(), nullptr, 0);

	// Draw
	gfx.pGetContext()->DrawIndexed(36, 0, 0);
}

void Cube::InitSharedResources(Graphics& gfx)
{
	HRESULT hr;


	THROW_FAILED_GFX(gfx.pGetDevice()->CreateVertexShader(shaders::VertexShaderBytecode,
		sizeof(shaders::VertexShaderBytecode), nullptr, &s_pVertexShader));


	// Pixel Shader


	THROW_FAILED_GFX(gfx.pGetDevice()->CreatePixelShader(shaders::PixelShaderBytecode,
		sizeof(shaders::PixelShaderBytecode), nullptr, &s_pPixelShader));

	// Create the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc),
		shaders::VertexShaderBytecode, sizeof(shaders::VertexShaderBytecode), &s_pInputLayout));
		
	// Define the cube vertices
	Vertex vertices[] =
	{
		// Front face
		{ { -1.0f, -1.0f, -1.0f }, {1, 0, 0, 1} },
		{ { -1.0f,  1.0f, -1.0f }, {0, 1, 0, 1} },
		{ {  1.0f,  1.0f, -1.0f }, {0, 0, 1, 1} },
		{ {  1.0f, -1.0f, -1.0f }, {1, 1, 1, 1} },
		// Back face
		{ { -1.0f, -1.0f,  1.0f }, {1, 1, 0, 1} },
		{ { -1.0f,  1.0f,  1.0f }, {0, 1, 1, 1} },
		{ {  1.0f,  1.0f,  1.0f }, {1, 0, 1, 1} },
		{ {  1.0f, -1.0f,  1.0f }, {0.5f, 0.5f, 0.5f, 1} },
	};

	D3D11_BUFFER_DESC vbDesc = CD3D11_BUFFER_DESC(sizeof(vertices),D3D11_BIND_VERTEX_BUFFER);

	D3D11_SUBRESOURCE_DATA vbSrd = {};
	vbSrd.pSysMem = vertices;

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&vbDesc, &vbSrd, &s_pVertexBuffer));

	// Create the index buffer
	WORD indices[] =
	{
		// Front face
		0,1,2, 0,2,3,
		// Back face
		4,6,5, 4,7,6,
		// Left face
		4,5,1, 4,1,0,
		// Right face
		3,2,6, 3,6,7,
		// Top face
		1,5,6, 1,6,2,
		// Bottom face
		4,0,3, 4,3,7,
	};

	D3D11_BUFFER_DESC ibDesc = CD3D11_BUFFER_DESC(sizeof(indices),D3D11_BIND_INDEX_BUFFER);

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&ibDesc, &ibData, &s_pIndexBuffer));
}