#pragma once
//
// Class for holding mouse status and event queue. Please note that the mouse position is only
// updated by mouse movement, and as such a "click" event will likely be off by a pixel or 2.
// This shouldn't be an issue since we use rawinput for first person camera movement, which is
// precise, but it's technically possible it could cause issues in a menu, in which case you
// should update it so that click events update the position.
//

#include "Win.h"
#include <DirectXMath.h>
#include <queue>
#include <optional>

class Mouse
{
	friend class Window;
public:
	class Event
	{
	public:
		enum Type
		{
			LeftDown,
			LeftUp,
			RightDown,
			RightUp,
			MiddleDown,
			MiddleUp,
			ScrollDown,
			ScrollUp,
			Move
		};
	public:
		Event( Type type, Mouse& mouse);
		Type GetType() const;
		int GetX() const;
		int GetY() const;
		std::pair<int, int> GetPos() const;
	private:
		Type type;
		int x;
		int y;
	};
public:
	Mouse();
	int GetX() const;
	int GetY() const;
	std::pair<int, int> GetPos() const;
	// Return the latest event and remove it from the event queue
	std::optional<Event> GetEvent();
	// Return the latest event without removing it from the event queue
	std::optional<Event> PeekEvent();
	bool LeftIsPressed() const;
	bool RightIsPressed() const;
	bool MiddleIsPressed() const;
private:
	void TrimQueue();
private:
	// ---Windows Called---
	void LButtonDown();
	void LButtonUp();
	void RButtonDown();
	void RButtonUp();
	void MButtonDown();
	void MButtonUp();
	void ScrollDown();
	void ScrollUp();
	void Movement( int in_x, int in_y );
private:
	static constexpr size_t queueLength = 8u;
	std::queue<Event> eventQueue;
	int x;
	int y;
	bool leftDown{ false };
	bool rightDown{ false };
	bool middleDown{ false };
};