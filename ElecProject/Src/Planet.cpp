#include "Planet.h"

Planet::Planet(Graphics& gfx, float patternseed, DirectX::XMFLOAT3 pos /*= { 0,0,0 }*/, float radius /*= 1.0f*/)
	: Sphere(gfx, patternseed, pos, {radius/2, radius/2, radius/2})
{

}
