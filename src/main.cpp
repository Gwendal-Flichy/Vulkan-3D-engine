
#include "VulkanCore.h"

int main()
{
	try
	{
		VulkanApplication app;
		app.run();
	}
	catch (const std::exception& e)
	{
		LOGE("%s", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}