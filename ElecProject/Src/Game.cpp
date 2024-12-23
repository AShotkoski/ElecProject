#include "Game.h"
#include <numbers>
#include "PhysEngine.h"
#include "ImGuiCustom.h"
#include "Ray.h"
#include "Logger.h"
#include <d3dcompiler.h>
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
		(float)gfx.GetWidth() / (float)gfx.GetHeight(), // Aspect ratio
		NearClipping, // near clipping
		FarClipping) // far clipping)
	);

	// Add planets
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, -1.f, dx::XMFLOAT3{ 0,0,0 }, 16.f));
	pPlanets.emplace_back(std::make_unique<Planet>(gfx, 0.5f, dx::XMFLOAT3{ 100,0,0 }, 7.0f));

	pPlanets[0]->SetVelocity({ 0, 0, 5.f });
	pPlanets[1]->SetVelocity({ 0, 6.f, -6.f });

	pPlanets[0]->SetMass(1e3);
	pPlanets[1]->SetMass(1);

	Logger::Get().OpenFile("output.csv");
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

	// Update logger time
	Logger::Get().UpdateTime(dt);

	gfx.EndFrame();
}

void Game::UpdateLogic()
{
	ControlCamera();

	if (isPhysicsEnabled)
		testPhys2();

	// Move planets
	if (controllingPlanet)
	{
		if (const auto mEvent = wnd.mouse.PeekEvent())
		{
			if (mEvent->GetType() == Mouse::Event::ScrollDown)
			{
				controlledPlanetDistAway -= 5.f;
			}
			if (mEvent->GetType() == Mouse::Event::ScrollUp)
			{
				controlledPlanetDistAway += 5.f;
			}
		}
		AttachPlanetToCursor();
	}

	while (const auto& mEvent = wnd.mouse.GetEvent())
	{ // mouse event
		if (mEvent->GetType() == Mouse::Event::LeftDown)
		{
			float xNDC = 2.f * (float)mEvent->GetX() / gfx.GetWidth() - 1.0f;
			float yNDC = 1.0f - 2.f * (float)mEvent->GetY() / gfx.GetHeight();
			auto optPlanet = DetectPlanetIntersection(xNDC, yNDC);
			if (optPlanet)
			{
				Planet& planet = optPlanet->get();

				controllingPlanet = true;
				controlledPlanet = &planet;

				// set the dist away
				auto cpos = gfx.GetCamera().GetPosition();
				controlledPlanetDistAway = dx::XMVectorGetX(dx::XMVector3LengthEst(dx::XMVectorSubtract(dx::XMLoadFloat3(&cpos), planet.GetVecPosition())));
			}
		}
		if (mEvent->GetType() == Mouse::Event::LeftUp)
		{
			if (controllingPlanet)
			{
				controllingPlanet = false;
				controlledPlanet = nullptr;
			}
		}
		if (mEvent->GetType() == Mouse::Event::RightDown)
		{
			// Convert pos to NDC
			float xNDC = 2.f * (float)mEvent->GetX() / gfx.GetWidth() - 1.0f;
			float yNDC = 1.0f - 2.f * (float)mEvent->GetY() / gfx.GetHeight();

			auto optPlanet = DetectPlanetIntersection(xNDC, yNDC);
			if (optPlanet)
			{
				Planet& planet = optPlanet->get();

				planet.ToggleControlWindow();
			}
		}
		
	}

	HandleKeyboardInput();

	SpawnControlWindow();
}

void Game::HandleKeyboardInput()
{

	// Keyboard Controls
	while (const auto& kbdEvent = wnd.kbd.GetEvent())
	{
		if (kbdEvent->GetType() == Keyboard::Event::Keydown)
		{
			float xNDC = 2.f * (float)wnd.mouse.GetX() / gfx.GetWidth() - 1.0f;
			float yNDC = 1.0f - 2.f * (float)wnd.mouse.GetY() / gfx.GetHeight();
			auto optPlanet = DetectPlanetIntersection(xNDC, yNDC);
			if (optPlanet)
			{
				Planet& planet = optPlanet->get();

				// handle key being pressed while mouse is over a planet
				switch (kbdEvent->GetVirtualKey())
				{
				case 'R':
					planet.SetVecVelocity(dx::XMVectorZero());
					break;
				case VK_DELETE:
				{
					auto it = std::find_if(pPlanets.begin(), pPlanets.end(), [&planet](const std::unique_ptr<Planet>& pl) {return pl.get() == &planet; });
					pPlanets.erase(it);
					break;
				}
				}

			}
		}
	}
}

void Game::SpawnControlWindow()
{
	ImGui::Begin("Game control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored({ 0.5f,0.1f,0,1 }, "There are %d planets", pPlanets.size());
	ImGui::InputFloat("G", &Gravitational_Const, 0.0f, 0.0f, "%e");
	ImGui::Checkbox("Physics", &isPhysicsEnabled);
	ImGui::InputFloat("Bounding Sphere Radius", &boundingSphereSize);

	if (ImGui::CollapsingHeader("New Planet"))
	{
		static float newPlanetMass = 1.f;
		static float newPlanetRadius = 10.f;
		ImGui::DragFloat("New Planet Mass", &newPlanetMass, 0.5f);
		ImGui::DragFloat("New Planet Radius", &newPlanetRadius, 0.1f);
		if (ImGui::Button("New Planet"))
		{
			auto midRay = RayUtils::fromNDC(0, 0, gfx.GetCamera().GetInvMatrix(), gfx.GetInvProjection());
			float newPlanetDistAway = newPlanetRadius * 2.f;
			auto newPlanetPos = dx::XMVectorAdd(midRay.origin, dx::XMVectorScale(midRay.direction, newPlanetDistAway));
			pPlanets.emplace_back(std::make_unique<Planet>(gfx, (float)rand(), dx::XMFLOAT3{ 0,0,0 }, newPlanetRadius));
			pPlanets.back()->SetVecPosition(newPlanetPos);
			pPlanets.back()->SetMass(newPlanetMass);
		}
	}

	if (ImGui::CollapsingHeader("Planet Grid"))
	{
		static float planetGridSpacing = 10.5f;
		static float planetGridRadius = 2.0f;
		static float planetGridMass = 1e-5f;

		ImGui::InputFloat("Spacing", &planetGridSpacing);
		ImGui::InputFloat("Radius", &planetGridRadius);
		ImGui::InputFloat("Mass", &planetGridMass);

		if (ImGui::Button("Create planet grid"))
		{
			CreatePlanetGrid(planetGridRadius, planetGridSpacing, planetGridMass);
		}
	}
	
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

	//ImGui::Begin("Vec2d selection");
	//float outx, outy;
	//ImGuiCustom::Vec2DInput("Test input ", &outx, &outy);

	//ImGui::End();
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
				DirectX::XMVECTOR forceV = agf.compute(s);
				auto accelV = dx::XMVectorScale(forceV, 1.0f / planetMasses[i]);
				// Clamp Acceleration
				constexpr const float maxAccel = 1e6;
				float magAccel = dx::XMVectorGetX(dx::XMVector3LengthEst(accelV));
				if (magAccel > maxAccel)
				{
					auto normAccel = dx::XMVector3Normalize(accelV);
					accelV = dx::XMVectorScale(normAccel, maxAccel);
				}
				return accelV;
			};
		// Acceleration lambda alt integration
		auto computeAccelBox = [&](const phys::State& s)
			{
				DirectX::XMVECTOR accel = dx::XMVectorZero();
				

				// Compute the distance from the origin to the object
				float dist;
				dx::XMStoreFloat(&dist, dx::XMVector3Length(s.position));

				if (dist > boundingSphereSize)
				{
					// Calculate the penetration depth
					float penetrationDepth = dist - boundingSphereSize;

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

std::optional<std::reference_wrapper<Planet>> Game::DetectPlanetIntersection(float ndcX, float ndcY)
{
	// Create the ray from the NDCs 
	const auto ray = RayUtils::fromNDC(ndcX, ndcY, gfx.GetCamera().GetInvMatrix(), gfx.GetInvProjection());

	std::optional<std::reference_wrapper<Planet>> intersected = std::nullopt;

	// Loop over the planets and detect collision (this is not efficient but it wont bottleneck us)
	for (const auto& p : pPlanets)
	{
		if (p->isRayIntersecting(ray))
		{
			intersected = std::ref(*p);
			break;
		}
	}

	return intersected;
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

void Game::AttachPlanetToCursor()
{
	using namespace DirectX;
	if (!controlledPlanet)
		return;
	float xNDC = 2.f * (float)wnd.mouse.GetX() / gfx.GetWidth() - 1.0f;
	float yNDC = 1.0f - 2.f * (float)wnd.mouse.GetY() / gfx.GetHeight();
	auto ray = RayUtils::fromNDC(xNDC, yNDC, gfx.GetCamera().GetInvMatrix(), gfx.GetInvProjection());
	controlledPlanet->SetVecPosition(ray.origin + XMVectorScale(ray.direction, controlledPlanetDistAway));
}

void Game::CreatePlanetGrid(float radius, float spacing, float planetMass)
{
	constexpr float r2o2 = 0.70710678f; // sqrt(2)/2 for bounding sphere diagonal estimation
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> udist(-150.f, 150.f);

	// Compute the maximum number of planets along one axis
	float cellSize = 2 * radius + spacing;
	size_t nX = static_cast<size_t>(boundingSphereSize * r2o2 * 2 / cellSize);

	for (size_t xi = 0; xi < nX; ++xi)
	{
		for (size_t yi = 0; yi < nX; ++yi)
		{
			for (size_t zi = 0; zi < nX; ++zi)
			{
				// Calculate position in the grid
				float xpos = -boundingSphereSize * r2o2 + xi * cellSize;
				float ypos = -boundingSphereSize * r2o2 + yi * cellSize;
				float zpos = -boundingSphereSize * r2o2 + zi * cellSize;

				// Check if the position is within the bounding sphere
				float distSquared = xpos * xpos + ypos * ypos + zpos * zpos;
				if (distSquared <= boundingSphereSize * boundingSphereSize)
				{
					// Create the planet if within bounds
					pPlanets.emplace_back(std::make_unique<Planet>(
						gfx,
						udist(rng),
						dx::XMFLOAT3{ xpos, ypos, zpos },
						radius
					));
					pPlanets.back()->SetMass(planetMass);
				}
			}
		}
	}
}

