#pragma once
#include "Window.h"
#include "FrameTimer.h"
#include "Planet.h"
#include <functional>
#include <optional>

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
	void HandleKeyboardInput();


	void SpawnControlWindow();

	void DrawFrame();
	// Wraps the if's for controlling the camera
	void ControlCamera();
	void AttachPlanetToCursor();

	// This function will create a grid of planets
	void CreatePlanetGrid(float radius, float spacing, float planetMass);

	// This function will be reworked at some point
	void testPhys2();
	float Gravitational_Const = 1e-2;
	float boundingSphereSize = 50.f;

	// If the normalized device coords are on a planet, return that planet
	// otherwise return an empty optional
	std::optional<std::reference_wrapper<Planet>> DetectPlanetIntersection(float ndcX, float ndcY);

private:
	Window wnd;
	Graphics& gfx;
	FrameTimer ft;
	std::vector<std::unique_ptr<Planet>> pPlanets;
private:
	float dt = 0;
	bool controllingPlanet = false;
	Planet* controlledPlanet = nullptr;
	float controlledPlanetDistAway = 12.f;
private:
	static constexpr UINT ScreenWidth = 1272u;
	static constexpr UINT ScreenHeight = 954u;
	static constexpr const wchar_t* WindowTitle = L"Rendering engine for Elec 1520";
	static constexpr float NearClipping = 0.1f;
	static constexpr float FarClipping = 1230.0f;
	static constexpr float Fov = 95.f; // degrees
	bool isPhysicsEnabled = false;
};
