#include "Game.h"
#include <numbers>
#include <d3dcompiler.h>
#include "PhysEngine.h"
#include "ThirdParty/ImGui/imgui.h"

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

	dx::XMFLOAT3 v0 = { 10.0f, 3.0f, -10.0f };
	pPlanets[0]->SetVelocity(dx::XMLoadFloat3(&v0));
	pPlanets[1]->SetMass(1.75e3);
	pPlanets[0]->SetMass(5e3);
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
	if (wnd.kbd.KeyIsPressed('K'))
		testPhys2();
}

void Game::DrawFrame()
{
	// Draw each planet
	for (auto& p : pPlanets)
	{
		p->Draw(gfx);
	}

	
}

void Game::testPhys2()
{
	auto getPlanetState = [](const Planet& p) -> phys::State
	{
			phys::State ret;
			ret.position = p.GetVecPosition();
			ret.velocity = p.GetVelocity();
			return ret;
	};

	phys::State planet = getPlanetState(*pPlanets[0]);
	//planet.position = pPlanets[0]->GetVecPosition();
	//planet.velocity = pPlanets[0]->GetVelocity();
	

	std::vector<phys::State> otherplanets;

	otherplanets.push_back(getPlanetState(*pPlanets[1]));

	std::vector<float> otherplanetmasses;
	otherplanetmasses.push_back(pPlanets[1]->GetMass());

	phys::GravForce gf(otherplanets, otherplanetmasses, 10, pPlanets[0]->GetMass());
	phys::AltGravForce agf(otherplanets, otherplanetmasses, 1, pPlanets[0]->GetMass());

	auto computeAccel = [&](const phys::State& s)
		{
			DirectX::XMVECTOR force = agf.compute(planet);
			return dx::XMVectorScale(force, 1.0f / pPlanets[0]->GetMass());
		};

	rk4Integrate(planet, dt, computeAccel);
	// Update the planet
	pPlanets[0]->SetVecPosition(planet.position);
	pPlanets[0]->SetVelocity(planet.velocity);
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
