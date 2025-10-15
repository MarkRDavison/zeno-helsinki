#include <cstdlib>
#include <iostream>

#include <helsinki/Example/ExampleConfiguration.hpp>
#include "ExampleConfig.hpp"

int main(int, char**)
{
	auto example = ex::ExampleConfiguration();

	// https://docs.vulkan.org/tutorial/latest/01_Overview.html
	// https://github.com/KhronosGroup/Vulkan-Tutorial/pull/119
	// https://github.com/KhronosGroup/Vulkan-Tutorial/tree/03d05cf0aaf4b321254721b652eaecbcb95ea09b/en/Building_a_Simple_Engine
	
	std::cout << "Root Path: " << ex::ExampleConfig::RootPath << std::endl;

	return EXIT_SUCCESS;
}