#pragma once
#include "ThirdParty/ImGui/imgui.h"
#include <map>
#include <cmath>

namespace ImGuiCustom
{

	// Creates a box that allows the user to draw a vector and outputs the resultant vector
	bool Vec2DInput(const char* label, float* out_x, float* out_y, const ImVec2& canvas_size = ImVec2(200, 200))
	{
		// Ensure a unique ID for each widget instance
		ImGui::PushID(label);

		// Static map to hold state per instance
		static std::map<ImGuiID, struct VectorInputState> state_map;

		ImGuiID id = ImGui::GetID(label);

		// Get the state for this instance
		struct VectorInputState
		{
			bool   isDragging = false;
			ImVec2 currentPos;
			ImVec2 velocity;
		};

		VectorInputState& state = state_map[id];

		// Begin the canvas area
		// Use the provided canvas size
		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImVec2 canvasSize = canvas_size;

		// Set a minimum size for the canvas
		if (canvasSize.x < 100.0f)
		{
			canvasSize.x = 100.0f;
		}
		if (canvasSize.y < 100.0f)
		{
			canvasSize.y = 100.0f;
		}

		// Draw canvas background and border
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(canvasPos,
			ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
			IM_COL32(50, 50, 50, 255));
		drawList->AddRect(canvasPos,
			ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
			IM_COL32(255, 255, 255, 255));

		// Add the label as a title on the canvas in small text at the top left
		drawList->AddText(ImVec2(canvasPos.x + 5, canvasPos.y + 5), IM_COL32(255, 255, 255, 255), label);

		// Add an invisible button to capture mouse events
		ImGui::InvisibleButton("canvas", canvasSize);

		// Check if the canvas is hovered
		bool isHoveringCanvas = ImGui::IsItemHovered();

		// Get mouse position relative to the canvas
		ImVec2 mousePosInCanvas = ImVec2(ImGui::GetIO().MousePos.x - canvasPos.x,
			ImGui::GetIO().MousePos.y - canvasPos.y);

		// Adjust origin to center of canvas
		ImVec2 canvasCenter = ImVec2(canvasSize.x * 0.5f, canvasSize.y * 0.5f);
		ImVec2 mousePosRelativeToCenter = ImVec2(mousePosInCanvas.x - canvasCenter.x,
			mousePosInCanvas.y - canvasCenter.y);

		// Clamp mousePosRelativeToCenter to stay within canvas boundaries
		ImVec2 maxPos = ImVec2(canvasCenter.x, canvasCenter.y);
		ImVec2 minPos = ImVec2(-canvasCenter.x, -canvasCenter.y);

		// Invert Y-axis so that up is positive
		mousePosRelativeToCenter.y = -mousePosRelativeToCenter.y;

		// Clamp the position
		mousePosRelativeToCenter.x = std::clamp(mousePosRelativeToCenter.x, minPos.x, maxPos.x);
		mousePosRelativeToCenter.y = std::clamp(mousePosRelativeToCenter.y, minPos.y, maxPos.y);

		// Handle input
		if (isHoveringCanvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			state.isDragging = true;
		}

		if (state.isDragging)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				state.isDragging = false;
				state.velocity = mousePosRelativeToCenter;
			}
			else
			{
				state.currentPos = mousePosRelativeToCenter;
				state.velocity = state.currentPos;
			}
		}

		// Function to check if a vector is approximately zero
		auto IsVectorZero = [](const ImVec2& vec)
			{
				const float threshold = 1e-6f;
				return (fabsf(vec.x) < threshold) && (fabsf(vec.y) < threshold);
			};

		// Draw the vector (either dragging or finalized)
		if (!IsVectorZero(state.velocity))
		{
			ImVec2 startPos = ImVec2(canvasPos.x + canvasCenter.x, canvasPos.y + canvasCenter.y);
			// Invert Y-axis back for drawing (since ImGui's Y-axis increases downwards)
			ImVec2 endPos = ImVec2(startPos.x + state.velocity.x,
				startPos.y - state.velocity.y);

			// Clamp endPos to stay within canvas boundaries
			endPos.x = std::clamp(endPos.x, canvasPos.x, canvasPos.x + canvasSize.x);
			endPos.y = std::clamp(endPos.y, canvasPos.y, canvasPos.y + canvasSize.y);

			ImU32 color = state.isDragging ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255);
			drawList->AddLine(startPos, endPos, color, 2.0f);

			// Draw arrowhead
			ImVec2 direction = state.velocity;
			float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
			if (length != 0.0f)
			{
				direction.x /= length;
				direction.y /= length;
			}

			// Adjust direction for drawing (invert Y-axis)
			ImVec2 directionDraw = ImVec2(direction.x, -direction.y);

			float arrowSize = 10.0f;
			ImVec2 arrowP1 = ImVec2(
				endPos.x - directionDraw.x * arrowSize - directionDraw.y * (arrowSize / 2),
				endPos.y - directionDraw.y * arrowSize + directionDraw.x * (arrowSize / 2));
			ImVec2 arrowP2 = ImVec2(
				endPos.x - directionDraw.x * arrowSize + directionDraw.y * (arrowSize / 2),
				endPos.y - directionDraw.y * arrowSize - directionDraw.x * (arrowSize / 2));
			drawList->AddTriangleFilled(endPos, arrowP1, arrowP2, color);
		}

		// Output the vector if there is one (either dragging or finalized)
		bool result = !IsVectorZero(state.velocity);
		if (result)
		{
			// Assign output values
			if (out_x) *out_x = state.velocity.x;
			if (out_y) *out_y = state.velocity.y;
		}
		else
		{
			if (out_x) *out_x = 0.0f;
			if (out_y) *out_y = 0.0f;
		}

		// Move cursor position after the canvas so that other widgets can be added below
		ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + canvasSize.y + ImGui::GetStyle().ItemSpacing.y));

		ImGui::PopID(); // Restore the ID stack

		return result;
	}


} // imguicustom namespace