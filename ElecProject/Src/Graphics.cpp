#include "Graphics.h"
#include "Macros.h"
#include <sstream>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d11sdklayers.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace WRL = Microsoft::WRL;
namespace dx = DirectX;

Graphics::Graphics(HWND hWnd)
	:
	projection(DirectX::XMMatrixIdentity())
{
	// Used for erro checking
	HRESULT hr;

	// Get window dimensions
	RECT clientRect;
	if (GetClientRect(hWnd, &clientRect) == 0)
		throw std::runtime_error("Error getting client rect.\n"); // todo graphics error
	Width = clientRect.right;
	Height = clientRect.bottom;


	// Setup SwapChain parameters
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferDesc.Width = Width;
	sd.BufferDesc.Height = Height;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = 0;

	// Use debug flag in d3ddevice creation if in debug mode
	UINT flags = 0u;
#ifndef NDEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Hold the created feature level
	D3D_FEATURE_LEVEL featureLevel;

	// Create d3d device and swap chain
	// Since feature levels array is blank, it uses default ordering
	THROW_FAILED_GFX(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr, // Feature level ptr
		0, // num elements in featuer level ptr
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		&featureLevel, // Out, feature level
		&pContext));

	// Ensure the required feature level is met for shader model 5.0
	if (featureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		throw std::runtime_error("The feature level of your graphics card does not meet the"
			" minimum specs for this program.");
	}


	// Get back buffer texture
	WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	THROW_FAILED_GFX(pSwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), &pBackBuffer));


	// Create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = sd.BufferDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
	THROW_FAILED_GFX(pDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, &pRenderTargetView));

	// Create the Depth stencil
	{
		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.Width = Width;
		depthStencilDesc.Height = Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		WRL::ComPtr<ID3D11Texture2D> pDSB;
		THROW_FAILED_GFX(pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &pDSB));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		THROW_FAILED_GFX(pDevice->CreateDepthStencilView(pDSB.Get(), &depthStencilViewDesc, &pDepthStencilView));

	}

	// Set the viewport
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (FLOAT)Width;
	vp.Height = (FLOAT)Height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	pContext->RSSetViewports(1u, &vp);
	// Bind the RTV
	pContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
}

Graphics::~Graphics()
{
}

void Graphics::BeginFrame()
{
	// Clear the RTV
	const FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f }; // Clear to black
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), clearColor);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	pContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
}

void Graphics::EndFrame()
{

	HRESULT hr;

	// Present back buffer
	if (FAILED(hr = pSwapChain->Present(enableVSync ? 1u : 0u, 0u)))
	{
		// Special throw case if graphics driver crashes
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			throw GFX_EXCEPT(hr);
		}
	}
}

UINT Graphics::GetWidth() const
{
	return Width;
}

UINT Graphics::GetHeight() const
{
	return Height;
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

ID3D11Device* Graphics::pGetDevice() const
{
	return pDevice.Get();
}

ID3D11DeviceContext* Graphics::pGetContext() const
{
	return pContext.Get();
}

ID3D11RenderTargetView* Graphics::pGetRTV() const
{
	return pRenderTargetView.Get();
}


Camera& Graphics::GetCamera()
{
	return camera;
}

DirectX::XMMATRIX Graphics::GetViewProjection() const
{
	return camera.GetMatrix() * GetProjection();
}

// -------------------------------------------------------------------------------
// ---------------------------GFX EXCEPTION---------------------------------------
// -------------------------------------------------------------------------------

Graphics::Exception::Exception(int line, const std::string& file, HRESULT hr)
	:
	BaseException(line, file),
	hr(hr)
{
}

const char* Graphics::Exception::what() const noexcept
{
	std::ostringstream ss;
	ss << BaseException::what() << std::endl
		<< "[Code] 0x" << std::hex << GetErrorCode() << std::dec << '(' << GetErrorCode() << ')' << std::endl
		<< "[Description] " << GetErrorString() << std::endl;
	whatBuffer = ss.str();
	return whatBuffer.c_str();
}

const char* Graphics::Exception::GetType() const noexcept
{
	return "Caught Graphics Exception:";
}

std::string Graphics::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr);
}

HRESULT Graphics::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::Exception::TranslateErrorCode(HRESULT hRes) noexcept
{
	char* pMsgBuffer = nullptr;
	DWORD dwMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hRes, 0,
		reinterpret_cast<LPSTR>(&pMsgBuffer),
		0u, nullptr
	);

	if (dwMsgLen == 0)
		return "Error code unknown.";

	std::string strMsg = pMsgBuffer;
	LocalFree(pMsgBuffer);

	return strMsg;
}
