#include <cstdlib>
#include <iostream>

#include <helsinki/System/SystemConfiguration.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Engine/EngineConfiguration.hpp>
#include <helsinki/Example/ExampleConfiguration.hpp>
#include <helsinki/System/Infrastructure/FileManager.hpp>

#include "ExampleConfig.hpp"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %d - %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

int main(int, char**)
{
	hl::FileManager fileManager;

	fileManager.registerDirectory(ex::ExampleConfig::RootPath);

	auto sys = hl::SystemConfiguration();
	auto ren = hl::RendererConfiguration();
	auto eng = hl::EngineConfiguration();
	auto example = ex::ExampleConfiguration();

	// https://docs.vulkan.org/tutorial/latest/01_Overview.html
	// https://github.com/KhronosGroup/Vulkan-Tutorial/pull/119
	// https://github.com/KhronosGroup/Vulkan-Tutorial/tree/03d05cf0aaf4b321254721b652eaecbcb95ea09b/en/Building_a_Simple_Engine
	// https://shader-slang.org/docs/coming-from-glsl/
	
	// https://github.com/ragulnathMB/VulkanProjectTemplate
	// https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
	// https://github.com/milkru/vulkanizer

	std::cout << "Root Path: " << ex::ExampleConfig::RootPath << std::endl;

	glm::vec2 vec{ 1,2 };

	std::cout << "Vec2: " << vec.x << "," << vec.y << std::endl;


	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		std::cerr << "Failed to initialise glfw" << std::endl;
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create glfw window" << std::endl;
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, key_callback);
	
	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return EXIT_SUCCESS;
}