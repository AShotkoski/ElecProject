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
	ControlCamera();
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

void Game::ControlCamera()
{
	DirectX::XMFLOAT3 dCampos = { 0, 0, 0 };

	if (wnd.kbd.KeyIsPressed('W'))
	{
		dCampos.z += 1.f;
	}
	if (wnd.kbd.KeyIsPressed('S'))
	{
		dCampos.z -= 1.f;
	}
	if (wnd.kbd.KeyIsPressed('A'))
	{
		dCampos.x -= 1.f;
	}
	if (wnd.kbd.KeyIsPressed('D'))
	{
		dCampos.x += 1.f;
	}
	if (wnd.kbd.KeyIsPressed('E'))
	{
		dCampos.y += 1.f;
	}
	if (wnd.kbd.KeyIsPressed('Q'))
	{
		dCampos.y -= 1.f;
	}
	// Control camera movement speed with scroll wheel
	while (auto e = wnd.mouse.GetEvent())
	{
		if (e->GetType() == Mouse::Event::ScrollUp)
		{
			gfx.GetCamera().UpdateMovementSpeed(1.05f);
		}
		else if (e->GetType() == Mouse::Event::ScrollDown)
		{
			gfx.GetCamera().UpdateMovementSpeed(0.95f);
		}
	}

	// Modify the camera if there has been user input
	if (dCampos.x != 0 || dCampos.y != 0 || dCampos.z != 0)
	{
		// Normalize camera movement vector so that diagonals are not twice as fast.
		DirectX::XMStoreFloat3(&dCampos, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dCampos)));

		gfx.GetCamera().UpdatePosition(dCampos, dt);
	}
}
