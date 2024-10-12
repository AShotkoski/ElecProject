#include "Game.h"
#include <numbers>
#include <d3dcompiler.h>

namespace dx = DirectX;
using namespace Microsoft::WRL;

Game::Game()
	: wnd(ScreenWidth, ScreenHeight, WindowTitle)
	, gfx(wnd.GFX())
{
	// Setup the projection matrix
	gfx.SetProjection(dx::XMMatrixPerspectiveFovLH(
		dx::XMConvertToRadians(Fov), // FOV
		(float) gfx.GetWidth() / (float)gfx.GetHeight(), // Aspect ratio
		NearClipping, // near clipping
		FarClipping) // far clipping)
	);

	// Add planets
	pPlanets.emplace_back(std::make_unique<Sphere>(gfx, 0.f, dx::XMFLOAT3{ 0,0,38 }, dx::XMFLOAT3{10,10,10}));
	pPlanets.emplace_back(std::make_unique<Sphere>(gfx, 0.5f, dx::XMFLOAT3{ -20,0,10 }, dx::XMFLOAT3{8,8,8}));
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
	if(wnd.kbd.KeyIsPressed('L'))
		testPhys();
}

void Game::DrawFrame()
{
	// Draw each planet
	for (auto& p : pPlanets)
	{
		p->Draw(gfx);
	}
}

void Game::ControlCamera()
{
	DirectX::XMFLOAT3 dCampos = { 0, 0, 0 };

	// Hold space to go into camera control mode
	if (wnd.kbd.KeyIsPressed(VK_SPACE))
	{
		gfx.GetCamera().EnableMouseControl();
		wnd.DisableCursor();
	}
	else
	{
		gfx.GetCamera().DisableMouseControl();
		wnd.EnableCursor();
		// Fast return and don't process keyboard input when not in mouse control mode,
		// be honest you don't need to move the camera in view mode.
		return;
	}

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

void Game::testPhys()
{
	// Velocity of planet at idx 0 (assuming velocity is stored and updated per object)
	static dx::XMFLOAT3 v0 = { -5.0f, 15.0f, 20.0f }; 

	// Gravitational constant (times masses, assuming it's scaled)
	constexpr float G = 190000.0f;

	// Get the positions of the planets
	dx::XMFLOAT3 p0 = pPlanets[0]->GetPos();
	dx::XMFLOAT3 p1 = pPlanets[1]->GetPos();

	// Load XMFLOAT3 into XMVECTORs
	dx::XMVECTOR vp0 = dx::XMLoadFloat3(&p0);
	dx::XMVECTOR vp1 = dx::XMLoadFloat3(&p1);

	// Calculate the vector between the two planets
	dx::XMVECTOR r01 = dx::XMVectorSubtract(vp1, vp0);

	// Calculate the squared distance between planets
	dx::XMVECTOR distsqVec = dx::XMVector3LengthSq(r01);
	float distsq;
	dx::XMStoreFloat(&distsq, distsqVec);

	if (distsq == 0.0f) {
		// Prevent division by zero if the planets are at the same position
		return;
	}

	// Calculate the unit vector (normalized direction) from p0 to p1
	dx::XMVECTOR r01hat = dx::XMVector3Normalize(r01);

	// Calculate the gravitational force magnitude
	dx::XMVECTOR vf0 = dx::XMVectorScale(r01hat, G / distsq);

	dx::XMVECTOR vDt = dx::XMVectorReplicate(dt);

	// Adjust force for time step
	vf0 = dx::XMVectorMultiply(vf0, vDt);

	// Update the velocity (assuming acceleration affects velocity)
	dx::XMVECTOR vVelocity0 = dx::XMLoadFloat3(&v0);
	vVelocity0 = dx::XMVectorAdd(vVelocity0, vf0);
	dx::XMStoreFloat3(&v0, vVelocity0);

	// Update the position using the new velocity
	vp0 = dx::XMVectorMultiplyAdd(vVelocity0, vDt, vp0);

	// Store the updated position
	dx::XMStoreFloat3(&p0, vp0);
	pPlanets[0]->setPosition(p0);

	
}
