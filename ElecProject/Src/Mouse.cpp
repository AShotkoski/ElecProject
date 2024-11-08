#include "Mouse.h"

Mouse::Event::Event( Type type, Mouse& mouse )
	: x(mouse.GetX())
	, y(mouse.GetY())
	, type(type)
{
	// Trim queue whenever an event is created in order to avoid duplicating code in each event push
	mouse.TrimQueue();
}

Mouse::Event::Type Mouse::Event::GetType() const
{
	return type;
}

int Mouse::Event::GetX() const
{
	return x;
}

int Mouse::Event::GetY() const
{
	return y;
}

std::pair<int, int> Mouse::Event::GetPos() const
{
	return std::make_pair(x,y);
}

// ---------- Main Mouse ----------

Mouse::Mouse()
	: x(0)
	, y(0)
{
}

int Mouse::GetX() const
{
	return x;
}

int Mouse::GetY() const
{
	return y;
}

std::pair<int, int> Mouse::GetPos() const
{
	return std::make_pair(x,y);
}

std::optional<Mouse::Event> Mouse::GetEvent()
{
	if ( eventQueue.empty() )
		return std::nullopt;

	auto front = eventQueue.front();
	eventQueue.pop();
	return front;
}

std::optional<Mouse::Event> Mouse::PeekEvent()
{
	if (eventQueue.empty())
		return std::nullopt;

	return eventQueue.front();
}

bool Mouse::LeftIsPressed() const
{
	return leftDown;
}

bool Mouse::RightIsPressed() const
{
	return rightDown;
}

bool Mouse::MiddleIsPressed() const
{
	return middleDown;
}

void Mouse::TrimQueue()
{
	while ( eventQueue.size() > queueLength )
		eventQueue.pop();
}

// ---Events--- 
void Mouse::LButtonDown()
{
	leftDown = true;
	eventQueue.push( Event( Event::LeftDown, *this ) );
}

void Mouse::LButtonUp()
{
	leftDown = false;
	eventQueue.push( Event( Event::LeftUp, *this ) );
}

void Mouse::RButtonDown()
{
	rightDown = true;
	eventQueue.push( Event( Event::RightDown, *this ) );
}

void Mouse::RButtonUp()
{
	rightDown = false;
	eventQueue.push( Event( Event::RightUp, *this ) );
}

void Mouse::MButtonDown()
{
	middleDown = true;
	eventQueue.push( Event( Event::MiddleDown, *this ) );
}

void Mouse::MButtonUp()
{
	middleDown = false;
	eventQueue.push( Event( Event::MiddleUp, *this ) );
}

void Mouse::ScrollDown()
{
	eventQueue.push( Event( Event::ScrollDown, *this ) );
}

void Mouse::ScrollUp()
{
	eventQueue.push( Event( Event::ScrollUp, *this ) );
}

void Mouse::Movement( int in_x, int in_y )
{
	x = in_x;
	y = in_y;
	eventQueue.push( Event( Event::Move, *this ) );
}
