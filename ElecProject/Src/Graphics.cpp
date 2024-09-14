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

Graphics::Graphics( HWND hWnd )
	:
	projection(DirectX::XMMatrixIdentity())
{
	// Used for erro chedcking
	HRESULT hr;
	
	// Get window dimensions
	RECT clientRect;
	if ( GetClientRect( hWnd, &clientRect ) == 0 )
		throw std::runtime_error( "Error getting client rect.\n" ); // todo graphics error
	Width = clientRect.right;
	Height = clientRect.bottom;


	// Setup SwapChain parameters
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferDesc.Width                   = Width;
	sd.BufferDesc.Height                  = Height;
	sd.BufferDesc.RefreshRate.Numerator   = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format                  = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count                   = 1;
	sd.SampleDesc.Quality                 = 0;
	sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount                        = 2;
	sd.OutputWindow                       = hWnd;
	sd.Windowed                           = true;
	sd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags                              = 0;

	// Use debug flag in d3ddevice creation if in debug mode
	UINT flags = 0u;
#ifndef NDEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create d3d device and swap chain
	THROW_FAILED_GFX( D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pContext ) );

	// Get back buffer tex
	WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	THROW_FAILED_GFX(pSwapChain->GetBuffer( 0u, __uuidof( ID3D11Texture2D ), &pBackBuffer  ));

	// Create render target view
	{
		// Create texture for RTV
		D3D11_TEXTURE2D_DESC tDesc = {};
		tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Using floats for pixels
		tDesc.ArraySize = 1;
		tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		tDesc.CPUAccessFlags = 0u;
		tDesc.Width = Width;
		tDesc.Height = Height;
		tDesc.MipLevels = 1;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.MiscFlags = 0u;
		WRL::ComPtr<ID3D11Texture2D> pTex;
		THROW_FAILED_GFX(pDevice->CreateTexture2D(&tDesc, nullptr, &pTex)); 

		// Create the view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = tDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
		THROW_FAILED_GFX(pDevice->CreateRenderTargetView(pTex.Get(), &rtvDesc, &pRenderTargetView));
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
	pContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), nullptr); // THERE IS NO DEPTH STENCIL ATM
}

Graphics::~Graphics()
{
}

void Graphics::BeginFrame()
{
	// Clear the RTV
	const FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f }; // Clear to black
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), clearColor);
}

void Graphics::Draw( UINT vertexCount, UINT start )
{
	pContext->Draw( vertexCount, start );
}

void Graphics::DrawIndexed( UINT indexCount )
{
	pContext->DrawIndexed( indexCount, 0u, 0u );
}

void Graphics::EndFrame()
{

	HRESULT hr;

	// Present back buffer
	if ( FAILED( hr = pSwapChain->Present( enableVSync ? 1u : 0u, 0u ) ) )
	{
		// Special throw case if graphics driver crashes
		if ( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw GFX_EXCEPT( pDevice->GetDeviceRemovedReason() );
		}
		else
		{
			throw GFX_EXCEPT( hr );
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

void Graphics::SetProjection( DirectX::FXMMATRIX proj ) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}


// -------------------------------------------------------------------------------
// ---------------------------GFX EXCEPTION---------------------------------------
// -------------------------------------------------------------------------------

Graphics::Exception::Exception( int line, const std::string& file, HRESULT hr )
	:
	BaseException( line, file ),
	hr( hr )
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
	return TranslateErrorCode( hr );
}

HRESULT Graphics::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::Exception::TranslateErrorCode( HRESULT hRes ) noexcept
{
	char* pMsgBuffer = nullptr;
	DWORD dwMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hRes, 0,
		reinterpret_cast<LPSTR>( &pMsgBuffer ),
		0u, nullptr
	);

	if ( dwMsgLen == 0 )
		return "Error code unknown.";

	std::string strMsg = pMsgBuffer;
	LocalFree( pMsgBuffer );

	return strMsg;
}
