// This class is kinda cheap lol 

#pragma once
#include "Planet.h"
#include <string>
#include <fstream>

class PlanetLogger
{
public:
	// attach a planet to the class, so that when log is called, the planet is logged
	void Attach(std::string filename, Planet* planetPtr, bool resetTimer = true)
	{
		// Remove old attachment
		if (isPlanetAttached())
			Detach();

		// Open file
		file.open(filename);
		assert(file && "failed to open file");
		
		// Reset timer if selected
		if (resetTimer)
			elapsedTime = 0;
		
		assert(planetPtr && "invalid pointer attached to planet logger");
		pPlanet = planetPtr;

		// Write the header
		file << "ElapsedTime, position.x, position.y, position.z" << std::endl;
	}
	void Log(float dt)
	{
		// Only proceed if a planet is attached
		if (!pPlanet)
			return; 

		elapsedTime += dt;
		const auto pPos = pPlanet->GetPosition();

		file << elapsedTime << ',';
		file << pPos.x << ',' << pPos.y << ',' << pPos.z << std::endl;

	}

	bool isPlanetAttached() const { return pPlanet; }

	void Detach()
	{
		file.close();
		pPlanet = nullptr;
	}
private:
	Planet* pPlanet = nullptr;
	float elapsedTime = 0;
	std::ofstream file;
};