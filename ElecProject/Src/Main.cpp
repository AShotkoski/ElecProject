#include "Window.h"
#include "BaseException.h"
#include "Game.h"

int WINAPI wWinMain(
	_In_ HINSTANCE     hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR         pCmdLine,
	_In_ int           nCmdShow )
{
	try
	{

		Game game;	// Create the game object, which will be where we program our demos and such

		while (true)
		{
			// processmessage returns a filled optional ( quit message )
			if (const auto exitCode = Window::ProcessMessage())
			{
				return *exitCode;
			}
			// Update game
			game.Go();
		}

	}
	catch ( const BaseException& e )
	{
		MessageBoxA( nullptr, e.what(), e.GetType(), 0u );
	}
	catch ( const std::exception& e )
	{
		MessageBoxA( nullptr, e.what(), "STL exception", 0u);
	}
	catch ( ... )
	{
		MessageBoxA( nullptr, "Unknown exception thrown", "unknown", 0u);
	}
	// If we get passed an exception
	return -1;
}
