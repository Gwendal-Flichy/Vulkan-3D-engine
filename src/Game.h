#pragma once

#include "RkSt_Window.h"

namespace RkSt
{
	class Game 
	{
	public:
		const std::string NAME = "Rock and Stone";
		const int WIDTH = 800;
		const int HEIGHT = 600;

		Game() = default;
		~Game() = default;

		void run();

	private:
		Window m_window{WIDTH,HEIGHT,NAME};
	};

}