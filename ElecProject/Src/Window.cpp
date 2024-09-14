#include "Window.h"
#include <sstream>
#include <hidusage.h>
#include <windowsx.h> 

// Setup singleton
Window::WindowClass Window::WindowClass::wndClass;


/******************   WINDOWS CLASS    ***********************/
Window::WindowClass::WindowClass()
	:
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wnd_class = { 0 };
	wnd_class.cbSize = sizeof( wnd_class );
	wnd_class.hInstance = GetHInstance();
	wnd_class.lpszClassName = ClassName;
	wnd_class.cbClsExtra = 0;
	wnd_class.cbWndExtra = 0;
	wnd_class.hbrBackground = nullptr;
	wnd_class.hCursor = nullptr;
	wnd_class.hIcon = static_cast<HICON>( LoadImage( hInst, nullptr, IMAGE_ICON, 64, 64, 0 ) );
	wnd_class.hIconSm = static_cast<HICON>( LoadImage( hInst, nullptr, IMAGE_ICON, 16, 16, 0 ) );;
	wnd_class.lpfnWndProc = Window::SetupMessageProc;
	wnd_class.lpszMenuName = 0;
	wnd_class.style = CS_OWNDC;

	RegisterClassEx( &wnd_class );
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass( ClassName, GetHInstance() );
}

HINSTANCE Window::WindowClass::GetHInstance() noexcept
{
	return wndClass.hInst;
}

std::wstring Window::WindowClass::GetName()  noexcept
{
	return std::wstring(ClassName);
}

/******************   WINDOWS CORE    ***********************/

Window::Window( UINT Width, UINT Height, const std::wstring& Title )
	:
	width(Width),
	height(Height)
{
	const DWORD dwCreationFlags = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	
	// Adjust creation dimensions to represent client area.
	RECT wndRect;
	wndRect.left   = 0;
	wndRect.top    = 0;
	wndRect.right  = wndRect.left + width;
	wndRect.bottom = wndRect.top + height;
	
	if ( AdjustWindowRectEx( &wndRect, dwCreationFlags, false, 0u ) == 0 )
	{
		throw WINDOW_LAST_ERROR_EXCEPT();
	}
	
	// Create Window, pass in pointer to this as lParam to use for message bs.
	hWnd = CreateWindowExW(
		0,
		WindowClass::GetName().c_str(),
		Title.c_str(),
		dwCreationFlags,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wndRect.right - wndRect.left,
		wndRect.bottom - wndRect.top,
		nullptr,
		nullptr,
		WindowClass::GetHInstance(),
		this );
	if ( hWnd == nullptr )
		throw WINDOW_LAST_ERROR_EXCEPT();

	// Init raw input devices (mouse)
	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage     = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags     = 0u;
	rid.hwndTarget  = nullptr;
	if ( RegisterRawInputDevices(&rid, 1u, sizeof(rid)) == FALSE )
	{
		throw WINDOW_LAST_ERROR_EXCEPT();
	}

	// Store where the center of the client area is in screen coords.
	
	POINT center = { (long)width / 2, (long)height / 2 };
	ClientToScreen( hWnd, &center );
	Center_x = center.x;
	Center_y = center.y;

	// Create Graphics object
	pGfx = std::make_unique<Graphics>( hWnd );

}

Window::~Window()
{
	DestroyWindow( hWnd );
}

std::optional<int> Window::ProcessMessage()
{
	// Message loop
	MSG msg;

	// Loop through all messages in queue(if any) and process them.
	while ( PeekMessage( &msg, nullptr, 0u, 0u, PM_REMOVE ) )
	{
		// For quit message return the error code
		if ( msg.message == WM_QUIT )
			return (int)msg.wParam;

		TranslateMessage( &msg );
		DispatchMessage( &msg );	
	}

	// Return empty optional
	return {};
}

Graphics& Window::GFX()
{
	return *pGfx;
}

RECT Window::GetRect() const
{
	RECT rect;
	if ( GetClientRect(hWnd, &rect) == FALSE )
		throw WINDOW_LAST_ERROR_EXCEPT();
	return rect;
}

float Window::GetAspectRatio() const
{
	return ((float)width / (float)height);
}

void Window::EnableCursor()
{
	// Only run once
	if ( cursorEnabled == true )
		return;

	cursorEnabled = true;
	ShowCursor();
	// stop clipping cursor
	if ( ClipCursor( nullptr ) == FALSE )
		throw WINDOW_LAST_ERROR_EXCEPT();
}

void Window::DisableCursor()
{
	// Only run once
	if ( cursorEnabled == false )
		return;

	cursorEnabled = false;
	HideCursor();
	// Confine cursor
	RECT wndRect = {};
	if(GetWindowRect( hWnd, &wndRect ) == FALSE)
		throw WINDOW_LAST_ERROR_EXCEPT();
	if ( ClipCursor( &wndRect ) == FALSE )
		throw WINDOW_LAST_ERROR_EXCEPT();
}

void Window::ShowCursor()
{
	while ( ::ShowCursor( true ) < 0 );
}

void Window::HideCursor()
{
	while ( ::ShowCursor( false ) >= 0 );
}

bool Window::isCursorEnabled() const
{
	return cursorEnabled;
}

LRESULT WINAPI Window::SetupMessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// This man has one purpose, to associate a ptr to the window class into the windows api.
	// Once done, messages will be sent to redirectmessageproc.
	
	// NCCREATE is called before window creation
	if ( msg == WM_NCCREATE )
	{
		// lParam will have a ptr to a CreateStruct
		const CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>( lParam );
		// pCreate holds the pointer to the window class inst we passed in on creation.
		Window* const pWnd = reinterpret_cast<Window*>( pCreate->lpCreateParams );
		// Store a ptr to the window class instance in the winapi
		SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pWnd ) );
		// Change to the redirect window proc
		SetWindowLongPtr( hWnd, GWLP_WNDPROC,
						  reinterpret_cast<LONG_PTR>(Window::RedirectMessageProc) );
		// Handle the message on messageproc as well
		return pWnd->MessageProc( hWnd, msg, wParam, lParam );
	}
	
	// let windows handle any sneaky window messages that happen to be before nccreate normally
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

LRESULT WINAPI Window::RedirectMessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// Retrieve window pointer
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr( hWnd, GWLP_USERDATA ));
	// Call message proc func
	return pWnd->MessageProc( hWnd, msg, wParam, lParam );
}

LRESULT Window::MessageProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

	// Main message switch
	switch ( msg )
	{
		// ---------Raw mouse input -----------
		case WM_INPUT:
		{
			// Test if mouse control is enabled for current gfx camera
			if ( !pGfx->GetCamera().isMouseControlEnabled() )
			{
				break;
			}

			UINT dwSize = 0u;

			// Get size of raw input and resize buffer to fit it
			GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER) );
			rawBuffer.resize( dwSize );
			// Fill raw buffer with data
			if ( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, rawBuffer.data(), &dwSize, sizeof(RAWINPUTHEADER))
				 != dwSize )
				throw WINDOW_LAST_ERROR_EXCEPT();
			const RAWINPUT* pRaw = (RAWINPUT*)rawBuffer.data();

			// Test if the raw data is mouse movement
			if ( pRaw->header.dwType == RIM_TYPEMOUSE && 
				 (pRaw->data.mouse.lLastX != 0 ||
				 pRaw->data.mouse.lLastY != 0))
			{
				// Directly update the camera to get as little input lag as possible
				pGfx->GetCamera().UpdateView( { (float)pRaw->data.mouse.lLastX,
											    (float)pRaw->data.mouse.lLastY } );

				SetCursorPos( Center_x, Center_y );
			}
			break;
		}
		// --------- Keyboard -----------
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if ( !( lParam & 0x40000000 ) )
			{
				kbd.Keydown( wParam );
			}
			break;
		case WM_KEYUP:
			kbd.Keyup( wParam );
			break;
		// ----------- Mouse Input -------------
		case WM_LBUTTONDOWN:
			mouse.LButtonDown();
			break;
		case WM_LBUTTONUP:
			mouse.LButtonUp();
			break;
		case WM_RBUTTONDOWN:
			mouse.RButtonDown();
			break;
		case WM_RBUTTONUP:
			mouse.RButtonUp();
			break;
		case WM_MBUTTONDOWN:
			mouse.MButtonDown();
			break;
		case WM_MBUTTONUP:
			mouse.MButtonUp();
			break;
		case WM_MOUSEMOVE:
			mouse.Movement( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
			break;
		case WM_MOUSEWHEEL:
		{
			const auto delta = GET_WHEEL_DELTA_WPARAM( wParam );
			if ( delta > 0 )
				mouse.ScrollUp();
			else
				mouse.ScrollDown();
			break;
		}

		// -----------   System    -------------
		case WM_MOVE:
		{
			POINT center = { (long)width / 2, (long)height / 2 };
			ClientToScreen( hWnd, &center );
			Center_x = center.x;
			Center_y = center.y;
			break;
		}
		case WM_CLOSE:
			PostQuitMessage( 0 );
			break;
	}
	
	// Let windows handle non user defined messages.
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

/******************   WINDOWS EXCEPTION    ***********************/
Window::Exception::Exception( int line, const std::string& file, HRESULT hr )
	:
	BaseException(line,file),
	hr(hr)
{
}

const char* Window::Exception::what() const noexcept
{
	std::ostringstream ss;
	ss << BaseException::what() << std::endl
		<< "[Code] " << GetErrorCode() << std::endl 
		<< "[Description] " << GetErrorString() << std::endl;
	whatBuffer = ss.str();
	return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept
{
	return "Caught Windows Exception:";
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr);
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::Exception::TranslateErrorCode( HRESULT hRes ) noexcept
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
