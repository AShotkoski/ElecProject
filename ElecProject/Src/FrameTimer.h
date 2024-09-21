#pragma once
#include <chrono>

class FrameTimer
{
public:
	FrameTimer();
	float Mark();
	float Peek() const;
	float GetTime() const;
private:
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point last;
	std::chrono::steady_clock::time_point old;
};