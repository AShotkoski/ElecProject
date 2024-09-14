#include "Game.h"
#include <numbers>

namespace dx = DirectX;

Game::Game()
	: wnd( ScreenWidth, ScreenHeight, WindowTitle )
	, gfx( wnd.GFX() )
{
	
}

Game::~Game()
{
}

void Game::Go()
{
	gfx.BeginFrame();
	UpdateLogic();
	DrawFrame();

	gfx.EndFrame();
}

void Game::UpdateLogic()
{	
	
}

void Game::DrawFrame()
{

}

