#pragma once

#include "Win.h"
#include "BaseException.h"
#include "Macros.h"
#include "Graphics.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <string>
#include <optional>
#include <memory>
#include <vector>

/******************   WINDOWS CLASS    ***********************/
class Window
{
public:
	/******************   WINDOWS EXCEPTION    ***********************/
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

private:
	// Singleton windows class to manage registration of window class
	class WindowClass
	{
	public:
		static HINSTANCE GetHInstance() noexcept;
		static std::wstring GetName() noexcept;
	private:
		WindowClass();
		~WindowClass();
		WindowClass( const WindowClass& ) = delete;
		WindowClass& operator=( const WindowClass& ) = delete;
	private:
		static WindowClass wndClass;
		HINSTANCE hInst;
		static constexpr const wchar_t* ClassName = L"Direct3D Windows Class";
	};
public:
	Window(UINT Width, UINT Height, const std::wstring& Title);
	~Window();
	Window( const Window& ) = delete;
	Window& operator=( const Window& ) = delete;
	// Exit code will be returned, otherwise empty optional
	static std::optional<int> ProcessMessage();
	Graphics& GFX();
	RECT GetRect() const;
	float GetAspectRatio() const;
	bool isCursorEnabled() const;
	void EnableCursor();
	void DisableCursor();
public:
	Keyboard kbd;
	Mouse mouse;
private:
	static LRESULT WINAPI SetupMessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT WINAPI RedirectMessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	LRESULT				  MessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void ShowCursor();
	void HideCursor();
private:
	std::vector<char> rawBuffer;
	UINT width;
	UINT height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
	bool cursorEnabled = true;
	int Center_x = 0;
	int Center_y = 0;
};

