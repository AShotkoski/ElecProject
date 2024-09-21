#include "Game.h"
#include <numbers>
#include <d3dcompiler.h>

namespace dx = DirectX;
using namespace Microsoft::WRL;

Game::Game()
	: wnd(ScreenWidth, ScreenHeight, WindowTitle)
	, gfx(wnd.GFX())
{

}

Game::~Game()
{
}

void Game::Go()
{
	dt = ft.Mark(); // Track frame time 
	gfx.BeginFrame();
	UpdateLogic();
	DrawFrame();

	gfx.EndFrame();
}

void Game::UpdateLogic()
{

}

void Game::DrawFrame()
{
	DrawTestCube();
}

void Game::DrawTestCube()
{
	HRESULT hr;

	// Vertex shader


	const char* vsCode =
		"cbuffer MatrixBuffer : register(b0)"
		"{"
		"    matrix worldViewProj;"
		"};"
		"struct VS_INPUT {"
		"    float3 pos : POSITION;"
		"    float4 color : COLOR;"
		"};"
		"struct PS_INPUT {"
		"    float4 pos : SV_POSITION;"
		"    float4 color : COLOR;"
		"};"
		"PS_INPUT VSMain(VS_INPUT input) {"
		"    PS_INPUT output;"
		"    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);"
		"    output.color = input.color;"
		"    return output;"
		"}";
	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> errBlob;
	if (FAILED(hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
		"VSMain", "vs_5_0", 0, 0, &vsBlob, &errBlob)))
	{
		if (errBlob.Get())
		{
			OutputDebugStringA((char*)errBlob->GetBufferPointer());
		}
		throw GFX_EXCEPT(hr);
	}

	ComPtr<ID3D11VertexShader> pVS;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), nullptr, &pVS));


	// Pixel Shader

	const char* psCode =
		"struct PS_INPUT {"
		"    float4 pos : SV_POSITION;"
		"    float4 color : COLOR;"
		"};"
		"float4 PSMain(PS_INPUT input) : SV_Target {"
		"    return input.color;"
		"}";

	ComPtr<ID3DBlob> psBlob;
	if (FAILED(hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
		"PSMain", "ps_5_0", 0, 0, &psBlob, &errBlob)))
	{
		if (errBlob.Get())
		{
			OutputDebugStringA((char*)errBlob->GetBufferPointer());
		}
		throw GFX_EXCEPT(hr);
	}

	ComPtr<ID3D11PixelShader> pPS;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), nullptr, &pPS));


	// Create the input layout 
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ComPtr<ID3D11InputLayout> pInputLayout;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),&pInputLayout));

	// DEFINE THE CUBE
	// Define the cube vertices
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

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

	// Create VB
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbSrd = {};
	vbSrd.pSysMem = vertices;

	ComPtr<ID3D11Buffer> pVertexBuffer;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&vbDesc, &vbSrd, &pVertexBuffer));

	// INDICES

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

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(indices);
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;

	ComPtr<ID3D11Buffer> pIndexBuffer;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&ibDesc, &ibData, &pIndexBuffer));

	// Create the constant buffer
	struct ConstantBuffer
	{
		DirectX::XMMATRIX worldViewProj;
	};

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	ComPtr<ID3D11Buffer> pConstBuffer;
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&cbDesc, nullptr, &pConstBuffer));


	dx::XMMATRIX view = dx::XMMatrixLookAtLH(
		dx::XMVectorSet(0.f, 0.f, -5.f, 1.f), // eye pos
		dx::XMVectorSet(0.f, 0.f, 0.f, 1.f), // focal pt
		dx::XMVectorSet(0.f, 1.f, 0.f, 0.f)); // up dir
	dx::XMMATRIX proj = dx::XMMatrixPerspectiveFovLH(
		dx::XM_PIDIV2, // fov
		ScreenWidth / ScreenHeight, // Aspect ratio
		NearClipping,
		FarClipping);

	static float theta = dt;
	dx::XMMATRIX world = dx::XMMatrixRotationRollPitchYaw(0, theta, 0.01f);
	theta += dt;

	dx::XMMATRIX worldviewproj = world * view * proj;


	worldviewproj = dx::XMMatrixTranspose(worldviewproj); // hlsl wants column major

	ConstantBuffer cb;
	cb.worldViewProj = worldviewproj;

	gfx.pGetContext()->UpdateSubresource(pConstBuffer.Get(), 0, nullptr, &cb, 0, 0);

	// Set everything

	gfx.pGetContext()->VSSetConstantBuffers(0, 1, pConstBuffer.GetAddressOf());
	gfx.pGetContext()->IASetInputLayout(pInputLayout.Get());
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	gfx.pGetContext()->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	gfx.pGetContext()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	gfx.pGetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.pGetContext()->VSSetShader(pVS.Get(), nullptr, 0);
	gfx.pGetContext()->PSSetShader(pPS.Get(), nullptr, 0);

	// Draw
	gfx.pGetContext()->DrawIndexed(36, 0, 0);



}

