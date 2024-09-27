#include "Game.h"
#include <numbers>
#include <d3dcompiler.h>

namespace dx = DirectX;
using namespace Microsoft::WRL;

Game::Game()
	: wnd(ScreenWidth, ScreenHeight, WindowTitle)
	, gfx(wnd.GFX())
	, cube(gfx, dx::XMFLOAT3{2.f, 0, 1.f})
	, cube2(gfx, {-2.f, 0, 0})
{
	// Setup the projection matrix
	gfx.SetProjection(dx::XMMatrixPerspectiveFovLH(
		dx::XMConvertToRadians(Fov), // FOV
		(float) gfx.GetWidth() / (float)gfx.GetHeight(), // Aspect ratio
		NearClipping, // near clipping
		FarClipping) // far clipping)
	);
}

Game::~Game()
{
}

void Game::Go()
{
	dt = ft.Mark(); // Track frame time 
	gfx.BeginFrame();
	UpdateLogic();
	DrawFrame();

	gfx.EndFrame();
}

void Game::UpdateLogic()
{
	if (wnd.kbd.KeyIsPressed(VK_SPACE))
		cube.setScaling(2.f);
	if (wnd.kbd.KeyIsPressed(VK_SPACE))
		cube2.setRotation(dx::XMQuaternionRotationRollPitchYaw(dt * 10.f, dt, 0));
}

void Game::DrawFrame()
{
	cube.Draw(gfx);
	cube2.Draw(gfx);
}
