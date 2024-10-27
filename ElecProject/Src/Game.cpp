#include "Game.h"
#include <numbers>
#include <d3dcompiler.h>
#include "PhysEngine.h"
#include "ImGuiCustom.h"
#include <random>

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
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 0.f, dx::XMFLOAT3{ 0,0,0 }, 18.f));
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 0.5f, dx::XMFLOAT3{ -20,0,10 }, 10.f));
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 1.25f, dx::XMFLOAT3{ -10,10,00 }, 10.f));

	pPlanets[0]->SetVelocity({ -1, 0, 1 });
	pPlanets[0]->SetMass(1800);
	pPlanets[1]->SetMass(1000);
	pPlanets[2]->SetMass(1000);
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

	if (isPhysicsEnabled)
		testPhys2();

	ImGui::Begin("test");
	ImGui::InputFloat("G", &Gravitational_Const, 0.0f, 0.0f, "%e");
	ImGui::Checkbox("Physics", &isPhysicsEnabled);
	ImGui::End();
}

void Game::DrawFrame()
{
	// Draw each planet
	for (auto& p : pPlanets)
	{
		p->Draw(gfx);
	}	
	wnd.GFX().GetCamera().spawnControlWindow();
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Vec2d selection");
	float outx, outy;
	ImGuiCustom::Vec2DInput("Test input ", &outx, &outy);
	
	float xNDC = outx / 100;
	float yNDC = outy / 100;

	DirectX::XMVECTOR rayDirView = dx::XMVectorSet(xNDC, yNDC, 1.f, 0.f);
	rayDirView = dx::XMVector3Normalize(rayDirView);
	DirectX::XMVECTOR rayDirWorld = dx::XMVector3TransformNormal(rayDirView, gfx.GetCamera().GetInvMatrix());
	rayDirWorld = dx::XMVector3Normalize(rayDirWorld);
	float dist = 50.f;
	dx::XMFLOAT3 camPosFl = gfx.GetCamera().GetPosition();
	DirectX::XMVECTOR worldCoords = dx::XMVectorAdd(dx::XMLoadFloat3(&camPosFl), dx::XMVectorScale(rayDirWorld, dist));
	
	// pPlanets[0]->SetVecPosition(worldCoords);
	
	ImGui::End();
}

void Game::testPhys2()
{
	auto getPlanetState = [](const Planet& p) -> phys::State
		{
			phys::State ret;
			ret.position = p.GetVecPosition();
			ret.velocity = p.GetVecVelocity();
			return ret;
		};

	size_t numPlanets = pPlanets.size();

	// Create vectors to hold the states and masses of all planets (super efficient)
	std::vector<phys::State> planetStates(numPlanets);
	std::vector<float> planetMasses(numPlanets);

	// Get all masses
	for (size_t i = 0; i < numPlanets; ++i)
	{
		planetStates[i] = getPlanetState(*pPlanets[i]);
		planetMasses[i] = pPlanets[i]->GetMass();
	}

	// Copy OG states 
	std::vector<phys::State> originalStates = planetStates;

	// For each planet, compute the acceleration due to other planets and integrate
	for (size_t i = 0; i < numPlanets; ++i)
	{
		// make vectors of other planets' states and masses
		std::vector<phys::State> otherStates;
		std::vector<float> otherMasses;

		for (size_t j = 0; j < numPlanets; ++j)
		{
			if (j != i)
			{
				otherStates.push_back(originalStates[j]);
				otherMasses.push_back(planetMasses[j]);
			}
		}

		// Gravitational force computation for planet i
		phys::GravForce agf(otherStates, otherMasses, Gravitational_Const, planetMasses[i]);

		// Acceleration lambda for integration
		auto computeAccel = [&](const phys::State& s)
			{
				DirectX::XMVECTOR force = agf.compute(s);
				return dx::XMVectorScale(force, 1.0f / planetMasses[i]);
			};
		// Acceleration lambda alt integration
		auto computeAccelBox = [&](const phys::State& s)
			{
				DirectX::XMVECTOR accel = dx::XMVectorZero();
				// Define the bounding sphere radius
				const float maxR = 50.0f;

				// Compute the distance from the origin to the object
				float dist;
				dx::XMStoreFloat(&dist, dx::XMVector3Length(s.position));

				if (dist > maxR)
				{
					// Calculate the penetration depth
					float penetrationDepth = dist - maxR;

					// Compute the normal vector pointing towards the center of the sphere
					DirectX::XMVECTOR normal = dx::XMVector3Normalize(s.position);

					// Apply an acceleration proportional to the penetration depth
					// Negative sign to push the object back inside the sphere
					accel = dx::XMVectorScale(normal, -penetrationDepth);

					// add damping to prevent oscillations
					const float dampingFactor = 0.5f;
					DirectX::XMVECTOR velocityAlongNormal = dx::XMVectorScale(normal, dx::XMVectorGetX(dx::XMVector3Dot(s.velocity, normal)));
					accel = dx::XMVectorSubtract(accel, dx::XMVectorScale(velocityAlongNormal, dampingFactor));
				}

				return accel;
			};

		// Integrate the state of planet i
		rk4Integrate(planetStates[i], dt, computeAccel);
		// Integrate for the bounding "box"
		rk4Integrate(planetStates[i], dt, computeAccelBox);
	}

	// Update the planets with their new positions and velocities
	for (size_t i = 0; i < numPlanets; ++i)
	{
		pPlanets[i]->SetVecPosition(planetStates[i].position);
		pPlanets[i]->SetVecVelocity(planetStates[i].velocity);
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
