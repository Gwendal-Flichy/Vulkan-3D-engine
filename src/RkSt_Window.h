#pragma once

#ifndef NDEBUG
# pragma comment(lib, "glfw3-s-d.lib")
#else /* NDEBUG */
# pragma comment(lib, "glfw3-s.lib")
#endif /* NDEBUG */

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// std
#include <string>

namespace RkSt
{
	class Window 
	{
	public:
		Window(const int& Width, const int& Height, const std::string& windowName);
		~Window();

		bool isOpen();

		int getWidth();
		const int getWidth() const;
		int getHeight();
		const int getHeight() const;

	private:
		void initWindow();

		int m_width;
		int m_height;
		std::string m_windowName;
		GLFWwindow* m_window;
	};

}// namespace RkSt