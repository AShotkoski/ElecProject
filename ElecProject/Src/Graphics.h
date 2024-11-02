#pragma once
#include "Win.h"
#include "BaseException.h"
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>
#include "Camera.h"

class Graphics
{
private:
	friend class GraphicsResource;	
public:
	class Exception : public BaseException
	{
	public:
		Exception( int line, const std::string& file, HRESULT hr );
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorString() const noexcept;
		HRESULT GetErrorCode() const noexcept;
		// Static function to translate error code, can be used statically
		static std::string TranslateErrorCode( HRESULT hRes ) noexcept;
	private:
		HRESULT hr;
	};
public:
	Graphics( HWND hWnd );
	~Graphics();
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
public:
	void BeginFrame();
	void EndFrame();
	UINT GetWidth() const;
	UINT GetHeight() const;
	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	ID3D11Device* pGetDevice() const;
	ID3D11DeviceContext* pGetContext() const;
	ID3D11RenderTargetView* pGetRTV() const;
	Camera& GetCamera();
	// Returns the View Projection matrix as a row-major matrix
	DirectX::XMMATRIX GetViewProjection() const;

private:
	DirectX::XMMATRIX projection;
	Camera camera;
	// Dimensions of client area
	UINT Width;
	UINT Height;
private:
	Microsoft::WRL::ComPtr<ID3D11Device>        pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>      pSwapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain> 		pRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;

private:
	/********************** Parameters *******************/
	static constexpr bool enableVSync = true;

};

