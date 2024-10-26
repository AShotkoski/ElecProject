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

	dx::XMFLOAT3 v0 = { -7.0f, 3.0f, 18.0f };
	pPlanets[0]->SetVelocity(dx::XMLoadFloat3(&v0));
	v0 = { 1.f,9.f,3.f };
	pPlanets[1]->SetVelocity(dx::XMLoadFloat3(&v0));
	v0 = { -1.f,-9.f,3.f };
	pPlanets[2]->SetVelocity(dx::XMLoadFloat3(&v0));
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


	ImGui::DragFloat("Drag G", &Gravitational_Const, 0.000001f, 0, 1000, "%.8f");
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
	//testDrawImGui();
	ImGui::Begin("Vec2d selection");
	float outx, outy;
	ImGuiCustom::Vec2DInput("Test input ", &outx, &outy);
	ImGuiCustom::Vec2DInput("Test input 2", &outx, &outy);
	ImGui::Text("(%f, %f)", outx, outy);
	ImGui::End();
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
		pPlanets[i]->SetVelocity(planetStates[i].velocity);
	}
}

void Game::testDrawImGui()
{
	struct VectorInput {
		bool isDragging = false;
		ImVec2 startPos;
		ImVec2 currentPos;
		ImVec2 velocity;
	} static vectorInput;

	// In your main loop
	ImGui::Begin("Vector Selector");

	// Obtain the canvas size and position
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();

	// Ensure the canvas has a minimum size
	if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
	if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;

	// Draw canvas background and border
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30, 30, 30, 255));
	drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(255, 255, 255, 255));

	// Add an invisible button over the canvas area to capture mouse events
	ImGui::InvisibleButton("canvas", canvasSize);

	// Check if the mouse is hovering over the canvas
	bool isHoveringCanvas = ImGui::IsItemHovered();

	// Get the mouse position relative to the canvas
	ImVec2 mousePosInCanvas = ImVec2(ImGui::GetIO().MousePos.x - canvasPos.x, ImGui::GetIO().MousePos.y - canvasPos.y);

	// Handle input
	if (isHoveringCanvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		vectorInput.isDragging = true;
		vectorInput.startPos = mousePosInCanvas;
		vectorInput.currentPos = mousePosInCanvas;
	}

	if (vectorInput.isDragging) {
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			vectorInput.isDragging = false;
			vectorInput.velocity = ImVec2(vectorInput.currentPos.x - vectorInput.startPos.x,
				vectorInput.currentPos.y - vectorInput.startPos.y);
		}
		else {
			vectorInput.currentPos = mousePosInCanvas;
		}
	}

	// Function to check if a vector is approximately zero
	auto IsVectorZero = [](const ImVec2& vec) {
		const float threshold = 1e-6f;
		return (fabsf(vec.x) < threshold) && (fabsf(vec.y) < threshold);
		};

	// Draw existing vector if it's not zero
	if (!IsVectorZero(vectorInput.velocity)) {
		ImVec2 startPos = ImVec2(canvasPos.x + vectorInput.startPos.x, canvasPos.y + vectorInput.startPos.y);
		ImVec2 endPos = ImVec2(startPos.x + vectorInput.velocity.x, startPos.y + vectorInput.velocity.y);
		drawList->AddLine(startPos, endPos, IM_COL32(0, 255, 0, 255), 2.0f);

		// Draw arrowhead
		ImVec2 direction = vectorInput.velocity;
		float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
		if (length != 0.0f) {
			direction.x /= length;
			direction.y /= length;
		}
		float arrowSize = 10.0f;
		ImVec2 arrowP1 = ImVec2(
			endPos.x - direction.x * arrowSize - direction.y * (arrowSize / 2),
			endPos.y - direction.y * arrowSize + direction.x * (arrowSize / 2)
		);
		ImVec2 arrowP2 = ImVec2(
			endPos.x - direction.x * arrowSize + direction.y * (arrowSize / 2),
			endPos.y - direction.y * arrowSize - direction.x * (arrowSize / 2)
		);
		drawList->AddTriangleFilled(endPos, arrowP1, arrowP2, IM_COL32(0, 255, 0, 255));
	}

	// Draw the dragging vector
	if (vectorInput.isDragging) {
		ImVec2 startPos = ImVec2(canvasPos.x + vectorInput.startPos.x, canvasPos.y + vectorInput.startPos.y);
		ImVec2 currentPos = ImVec2(canvasPos.x + vectorInput.currentPos.x, canvasPos.y + vectorInput.currentPos.y);
		drawList->AddLine(startPos, currentPos, IM_COL32(255, 0, 0, 255), 2.0f);

		// Draw arrowhead
		ImVec2 direction = ImVec2(vectorInput.currentPos.x - vectorInput.startPos.x,
			vectorInput.currentPos.y - vectorInput.startPos.y);
		float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
		if (length != 0.0f) {
			direction.x /= length;
			direction.y /= length;
		}
		float arrowSize = 10.0f;
		ImVec2 endPos = currentPos;
		ImVec2 arrowP1 = ImVec2(
			endPos.x - direction.x * arrowSize - direction.y * (arrowSize / 2),
			endPos.y - direction.y * arrowSize + direction.x * (arrowSize / 2)
		);
		ImVec2 arrowP2 = ImVec2(
			endPos.x - direction.x * arrowSize + direction.y * (arrowSize / 2),
			endPos.y - direction.y * arrowSize - direction.x * (arrowSize / 2)
		);
		drawList->AddTriangleFilled(endPos, arrowP1, arrowP2, IM_COL32(255, 0, 0, 255));
	}

	ImGui::End();

	// Display vector data
	if (!IsVectorZero(vectorInput.velocity)) {
		ImGui::Begin("Vector Data");
		ImGui::Text("Initial Velocity:");
		ImGui::Text("X: %.2f", vectorInput.velocity.x);
		ImGui::Text("Y: %.2f", vectorInput.velocity.y);
		ImGui::End();
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
