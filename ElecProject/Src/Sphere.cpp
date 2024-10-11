#include "Sphere.h"
#include "Macros.h"
#include <unordered_map>
#include <algorithm>
namespace shaders
{
#include "PixelShader.shaderheader" // If there is an error, ignore it. File gets created on compilation
#include "VertexShader.shaderheader"
}
// Namespace definitions in the implementation file
using namespace Microsoft::WRL;
namespace dx = DirectX;

// Initialize the static members globally
ComPtr<ID3D11VertexShader> Sphere::s_pVertexShader;
ComPtr<ID3D11PixelShader> Sphere::s_pPixelShader;
ComPtr<ID3D11InputLayout> Sphere::s_pInputLayout;
ComPtr<ID3D11Buffer> Sphere::s_pVertexBuffer;
ComPtr<ID3D11Buffer> Sphere::s_pIndexBuffer;
UINT Sphere::indCount = 0u;
bool Sphere::s_sharedResourcesInitialized = false;


Sphere::Sphere(Graphics& gfx, float patternSeed, dx::XMFLOAT3 pos, dx::XMFLOAT3 scale, dx::FXMVECTOR rotation)
	:
	position(pos),
	scaling(scale),
	rotation(rotation)
{
	HRESULT hr;
	// Initialize the shared resources if they are not yet initialized 
	if (!s_sharedResourcesInitialized)
	{
		indCount = InitSharedResources(gfx);
		s_sharedResourcesInitialized = true;
	}

	// Create the instance specific const buffer (using the cd3d11 helpers here since 
	// I remembered they existed and they make the code less verbose)
	D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(sizeof(ConstBuffer), D3D11_BIND_CONSTANT_BUFFER);
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&cbDesc, nullptr, &pConstBuffer));
	// Default the const buffer to identity matrices
	ConstBuffer.worldViewProj = dx::XMMatrixIdentity();
	ConstBuffer.world = dx::XMMatrixIdentity();
	// set the CB seed
	ConstBuffer.perlinRNGSeed = patternSeed;

	// Update the constant buffer for the passed in params
	updateCB();

}


void Sphere::Draw(Graphics& gfx)
{

	ConstBuffer.worldViewProj = dx::XMMatrixTranspose(ConstBuffer.world * gfx.GetViewProjection());
	// Update the CB
	gfx.pGetContext()->UpdateSubresource(pConstBuffer.Get(), 0, nullptr, &ConstBuffer, 0, 0);

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
	gfx.pGetContext()->DrawIndexed(indCount, 0, 0);
}


void Sphere::setPosition(DirectX::XMFLOAT3 newPos)
{
	position = newPos;
	updateCB();
}

void Sphere::setScaling(DirectX::XMFLOAT3 newScaling)
{
	scaling = newScaling;
	updateCB();
}

void Sphere::setScaling(float factor)
{
	scaling = dx::XMFLOAT3(factor, factor, factor);
	updateCB();
}

void Sphere::setRotation(DirectX::FXMVECTOR quaternion)
{
	rotation = quaternion;
	updateCB();
}

DirectX::XMFLOAT3 Sphere::GetPos() const
{
	return position;
}

UINT Sphere::InitSharedResources(Graphics& gfx)
{
	HRESULT hr;

	// Vertex Shader
	THROW_FAILED_GFX(gfx.pGetDevice()->CreateVertexShader(shaders::VertexShaderBytecode,
		sizeof(shaders::VertexShaderBytecode), nullptr, &s_pVertexShader));


	// Pixel Shader
	THROW_FAILED_GFX(gfx.pGetDevice()->CreatePixelShader(shaders::PixelShaderBytecode,
		sizeof(shaders::PixelShaderBytecode), nullptr, &s_pPixelShader));

	// Create the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		  0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc),
		shaders::VertexShaderBytecode, sizeof(shaders::VertexShaderBytecode), &s_pInputLayout));

	// Generate the cube verts and inds to make those buffers
	std::vector<Vertex> vertBuffer;
	std::vector<unsigned short> indBuffer;
	// The number of triangles in the mesh is given by 
	// T = 20*4^n where n is the subdivisions. Keep this in mind
	GenerateGeometry(3, vertBuffer, indBuffer);

	D3D11_BUFFER_DESC vbDesc = CD3D11_BUFFER_DESC((UINT)vertBuffer.size() * sizeof(Vertex), D3D11_BIND_VERTEX_BUFFER);

	D3D11_SUBRESOURCE_DATA vbSrd = {};
	vbSrd.pSysMem = vertBuffer.data();

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&vbDesc, &vbSrd, &s_pVertexBuffer));


	D3D11_BUFFER_DESC ibDesc = CD3D11_BUFFER_DESC((UINT)indBuffer.size() * sizeof(unsigned short), D3D11_BIND_INDEX_BUFFER);

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indBuffer.data();

	THROW_FAILED_GFX(gfx.pGetDevice()->CreateBuffer(&ibDesc, &ibData, &s_pIndexBuffer));

	return (UINT)indBuffer.size();
}

void Sphere::updateCB()
{
	ConstBuffer.world = dx::XMMatrixRotationQuaternion(rotation) *
		dx::XMMatrixScaling(scaling.x, scaling.y, scaling.z) *
		dx::XMMatrixTranslation(position.x, position.y, position.z);
}

void Sphere::GenerateGeometry(size_t subdivisions, std::vector<Vertex>& out_vertices, std::vector<unsigned short>& out_indices)
{
	out_vertices.clear();
	out_indices.clear();

	// Initial icosahedron vertices
	constexpr float phi = 1.61803398875f; // (1 + sqrt(5)) / 2, the golden ratio

	// Add initial vertices (normalized to the unit sphere)
	std::vector<DirectX::XMFLOAT3> initial_positions = 
	{
		{-1.0f,  phi,  0.0f}, { 1.0f,  phi,  0.0f}, {-1.0f, -phi,  0.0f}, { 1.0f, -phi,  0.0f},
		{ 0.0f, -1.0f,  phi}, { 0.0f,  1.0f,  phi}, { 0.0f, -1.0f, -phi}, { 0.0f,  1.0f, -phi},
		{ phi,  0.0f, -1.0f}, { phi,  0.0f,  1.0f}, {-phi,  0.0f, -1.0f}, {-phi,  0.0f,  1.0f}
	};

	auto Normalize = [](DirectX::XMFLOAT3& v) 
		{
		float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		v.x /= length;
		v.y /= length;
		v.z /= length;
		};

	for (auto& pos : initial_positions) 
	{
		Normalize(pos);
		Vertex v = {};
		v.position = pos;
		out_vertices.emplace_back(v);
	}

	// Initial icosahedron indices (clockwise winding order)
	out_indices = 
	{
		0, 11, 5,   0, 5, 1,    0, 1, 7,    0, 7, 10,   0, 10, 11,
		1, 5, 9,    5, 11, 4,   11, 10, 2,  10, 7, 6,   7, 1, 8,
		3, 9, 4,    3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
		4, 9, 5,    2, 4, 11,   6, 2, 10,   8, 6, 7,    9, 8, 1
	};

	// Subdivision function
	auto GetEdgeKey = [](unsigned short a, unsigned short b) 
		{
		return ((uint64_t)std::min(a, b) << 32) | std::max(a, b);
		};

	// Lambda for getting the midpoints
	auto GetMidpoint = [&](unsigned short a, unsigned short b, std::unordered_map<uint64_t, unsigned short>& cache) 
		{
		uint64_t key = GetEdgeKey(a, b);
		auto it = cache.find(key);
		if (it != cache.end()) {
			return it->second;
		}

		// Calculate midpoint and normalize to the sphere surface
		const DirectX::XMFLOAT3& posA = out_vertices[a].position;
		const DirectX::XMFLOAT3& posB = out_vertices[b].position;
		DirectX::XMFLOAT3 mid_pos = {
			(posA.x + posB.x) * 0.5f,
			(posA.y + posB.y) * 0.5f,
			(posA.z + posB.z) * 0.5f
		};
		Normalize(mid_pos);

		// Compute normal and texture coordinates
		Vertex v = {};
		v.position = mid_pos;

		out_vertices.push_back(v);
		unsigned short index = static_cast<unsigned short>(out_vertices.size() - 1);
		cache[key] = index;
		return index;
		};

	// Subdivide triangles
	for (size_t i = 0; i < subdivisions; ++i) 
	{
		std::unordered_map<uint64_t, unsigned short> midpoints_cache;
		std::vector<unsigned short> new_indices;

		for (size_t j = 0; j < out_indices.size(); j += 3) 
		{
			unsigned short a = out_indices[j];
			unsigned short b = out_indices[j + 1];
			unsigned short c = out_indices[j + 2];

			// Get midpoints and avoid duplication
			unsigned short ab = GetMidpoint(a, b, midpoints_cache);
			unsigned short bc = GetMidpoint(b, c, midpoints_cache);
			unsigned short ca = GetMidpoint(c, a, midpoints_cache);

			// Form new triangles
			new_indices.insert(new_indices.end(), { a, ab, ca });
			new_indices.insert(new_indices.end(), { b, bc, ab });
			new_indices.insert(new_indices.end(), { c, ca, bc });
			new_indices.insert(new_indices.end(), { ab, bc, ca });
		}

		
		out_indices = std::move(new_indices);
	}

}
