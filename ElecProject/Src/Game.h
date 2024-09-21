#pragma once
#include "Window.h"
#include "FrameTimer.h"

class Game
{
public:
	Game();
	~Game();
	Game( const Game& ) = delete;
	const Game& operator=( const Game& ) = delete;
	void Go();
private:
	void UpdateLogic();
	void DrawFrame();

	void DrawTestCube();

private:
	Window wnd;
	Graphics& gfx;
	FrameTimer ft;

private:
	float dt = 0;
private:
	static constexpr UINT ScreenWidth = 1272u;
	static constexpr UINT ScreenHeight = 954u;
	static constexpr const wchar_t* WindowTitle = L"Rendering engine";
	static constexpr float NearClipping = 0.1f;
	static constexpr float FarClipping = 130.0f;
	static constexpr float Fov = 80.f;
};

