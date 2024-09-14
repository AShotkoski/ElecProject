#include "Keyboard.h"

// Windows-called funcs

bool Keyboard::KeyIsPressed( unsigned char VirtualKey ) const
{
	return keystates[VirtualKey];
}

void Keyboard::TrimQueue()
{
	while ( eventQueue.size() > queueLength )
		eventQueue.pop();
}

std::optional<Keyboard::Event> Keyboard::GetEvent()
{
	if ( eventQueue.empty() )
		return std::nullopt;

	auto front = eventQueue.front();
	eventQueue.pop();
	return front;
}

Keyboard::Event::Type Keyboard::Event::GetType() const
{
	return type;
}

void Keyboard::Keydown( WPARAM VK )
{
	// Set true on keystate at virtual key index
	keystates[VK] = true;
	// Add event of type keydown to event queue
	eventQueue.push( Event( Event::Keydown, static_cast<unsigned char>(VK), *this ) );
}

void Keyboard::Keyup( WPARAM VK )
{
	keystates[VK] = false;
	eventQueue.push( Event( Event::Keyup, static_cast<unsigned char>( VK ), *this ) );
}

Keyboard::Event::Event( Type type, unsigned char VK, Keyboard& kbd )
	: type(type)
	, VK(VK)
{
	kbd.TrimQueue();
}

unsigned char Keyboard::Event::GetVirtualKey() const
{
	return VK;
}
