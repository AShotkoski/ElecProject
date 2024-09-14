#pragma once
#include "Win.h"
#include <bitset>
#include <queue>
#include <optional>

class Keyboard
{
	friend class Window;
public:
	class Event
	{
	public:
		enum Type
		{
			Keyup,
			Keydown
		};
		Event( Type type, unsigned char VK, Keyboard& kbd);
		unsigned char GetVirtualKey() const;
		Type GetType() const;
	private:
		Type type;
		unsigned char VK;
	};
public:
	Keyboard() = default;
	bool KeyIsPressed( unsigned char VirtualKey ) const;
	std::optional<Event> GetEvent();
private:
	void TrimQueue();
private:
	// Windows-called funcs
	void Keydown( WPARAM VK );
	void Keyup( WPARAM VK );
private:
	std::bitset<256u> keystates;
	static constexpr size_t queueLength = 8u;
	std::queue<Event> eventQueue;
};

