#include "Game.h"


namespace RkSt
{
	void Game::run()
	{
		while (m_window.isOpen())
		{
			glfwPollEvents();
		}
	}

}// namespace RkSt


