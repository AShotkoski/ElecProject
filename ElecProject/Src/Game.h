#pragma once
#include "Window.h"
#include "FrameTimer.h"
#include "Planet.h"
#include <functional>

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
	// Wraps the if's for controlling the camera
	void ControlCamera();

	// This function will be reworked at some point
	void testPhys2();
	float Gravitational_Const = 1.5e1;

private:
	Window wnd;
	Graphics& gfx;
	FrameTimer ft;
	std::vector<std::unique_ptr<Planet>> pPlanets;

private:
	float dt = 0;
private:
	static constexpr UINT ScreenWidth = 1272u;
	static constexpr UINT ScreenHeight = 954u;
	static constexpr const wchar_t* WindowTitle = L"Rendering engine for Elec 1520";
	static constexpr float NearClipping = 0.1f;
	static constexpr float FarClipping = 1300.0f;
	static constexpr float Fov = 103.f; // degrees
	bool isPhysicsEnabled = false;
};
