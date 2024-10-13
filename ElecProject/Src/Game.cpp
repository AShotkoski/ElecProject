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
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 0.f, dx::XMFLOAT3{ 0,0,38 }, 18.f));
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 0.5f, dx::XMFLOAT3{ -20,0,10 }, 10.f));
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
	auto& planet = pPlanets[0];

	// Set initial velocity
	dx::XMFLOAT3 v0 = { 0.0f, 0.0f, 0.0f };
	planet->SetVelocity(dx::XMLoadFloat3(&v0));

	// Gravitational constant (G)
	const float G = 10000.f;

	// Masses of the planets
	float mass0 = planet->GetMass();
	float mass1 = pPlanets[1]->GetMass();


	// Get positions of the planets
	dx::XMFLOAT3 p0 = planet->GetPosition();
	dx::XMFLOAT3 p1 = pPlanets[1]->GetPosition();

	dx::XMVECTOR pos0 = dx::XMLoadFloat3(&p0);
	dx::XMVECTOR pos1 = dx::XMLoadFloat3(&p1);
	dx::XMVECTOR vel0 = planet->GetVelocity();

	// Lambda to compute gravitational force between two positions
	auto computeGravitationalForce = [&](dx::XMVECTOR posA, dx::XMVECTOR posB) -> dx::XMVECTOR {
		dx::XMVECTOR r = dx::XMVectorSubtract(posB, posA);
		dx::XMVECTOR distSqVec = dx::XMVector3LengthSq(r);
		float distSq;
		dx::XMStoreFloat(&distSq, distSqVec);
		if (distSq == 0.0f) {
			return dx::XMVectorZero(); // Prevent division by zero
		}
		dx::XMVECTOR rHat = dx::XMVector3Normalize(r);
		float forceMag = G * mass0 * mass1 / distSq;
		dx::XMVECTOR forceVec = dx::XMVectorScale(rHat, forceMag);
		return forceVec;
		};

	// Compute gravitational force at initial position
	dx::XMVECTOR force0 = computeGravitationalForce(pos0, pos1);

	// k1 calculations
	dx::XMVECTOR k1_p = vel0;
	dx::XMVECTOR k1_v = planet->calcAcceleration(force0);

	// k2 calculations
	dx::XMVECTOR pos_k2 = dx::XMVectorAdd(pos0, dx::XMVectorScale(k1_p, dt / 2.0f));
	dx::XMVECTOR vel_k2 = dx::XMVectorAdd(vel0, dx::XMVectorScale(k1_v, dt / 2.0f));
	dx::XMVECTOR force_k2 = computeGravitationalForce(pos_k2, pos1);
	dx::XMVECTOR k2_p = vel_k2;
	dx::XMVECTOR k2_v = planet->calcAcceleration(force_k2);

	// k3 calculations
	dx::XMVECTOR pos_k3 = dx::XMVectorAdd(pos0, dx::XMVectorScale(k2_p, dt / 2.0f));
	dx::XMVECTOR vel_k3 = dx::XMVectorAdd(vel0, dx::XMVectorScale(k2_v, dt / 2.0f));
	dx::XMVECTOR force_k3 = computeGravitationalForce(pos_k3, pos1);
	dx::XMVECTOR k3_p = vel_k3;
	dx::XMVECTOR k3_v = planet->calcAcceleration(force_k3);

	// k4 calculations
	dx::XMVECTOR pos_k4 = dx::XMVectorAdd(pos0, dx::XMVectorScale(k3_p, dt));
	dx::XMVECTOR vel_k4 = dx::XMVectorAdd(vel0, dx::XMVectorScale(k3_v, dt));
	dx::XMVECTOR force_k4 = computeGravitationalForce(pos_k4, pos1);
	dx::XMVECTOR k4_p = vel_k4;
	dx::XMVECTOR k4_v = planet->calcAcceleration(force_k4);

	// Update velocity
	dx::XMVECTOR vel_new = dx::XMVectorAdd(vel0, dx::XMVectorScale(
		dx::XMVectorAdd(
			dx::XMVectorAdd(k1_v, dx::XMVectorScale(k2_v, 2.0f)),
			dx::XMVectorAdd(dx::XMVectorScale(k3_v, 2.0f), k4_v)
		), dt / 6.0f));

	// Update position
	dx::XMVECTOR pos_new = dx::XMVectorAdd(pos0, dx::XMVectorScale(
		dx::XMVectorAdd(
			dx::XMVectorAdd(k1_p, dx::XMVectorScale(k2_p, 2.0f)),
			dx::XMVectorAdd(dx::XMVectorScale(k3_p, 2.0f), k4_p)
		), dt / 6.0f));

	// Update planet's state
	planet->SetVelocity(vel_new);
	planet->SetVecPosition(pos_new);
}
