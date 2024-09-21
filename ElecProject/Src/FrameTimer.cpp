#include "FrameTimer.h"

using namespace std::chrono;

FrameTimer::FrameTimer()
{
	last = steady_clock::now();
	begin = last;
}

// returns time since last call, or since if no call has been made ctor
float FrameTimer::Mark() 
{
	old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

float FrameTimer::Peek() const
{
	auto curr = steady_clock::now();
	const duration<float> frameTime = curr - old;
	return frameTime.count();
}

float FrameTimer::GetTime() const
{
	auto curr = steady_clock::now();
	const duration<float> Time = curr - begin;
	return Time.count();
}
