#include "RkSt_Window.h"


namespace RkSt 
{
	
	Window::Window(const int& Width, const int& Height, const std::string& windowName) 
		:m_width(Width),
		m_height(Height),
		m_windowName(windowName)
	{
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
	}

	bool Window::isOpen()
	{
		return !glfwWindowShouldClose(m_window);
	}

	int Window::getWidth()
	{
		return m_width;
	}

	const int Window::getWidth() const
	{
		return m_width;
	}

	int Window::getHeight() 
	{
		return m_height;
	}

	const int Window::getHeight() const
	{
		return m_height;
	}

} //namespace RkSt