#include <cstdlib>
#include <iostream>

#include <helsinki/Example/ExampleConfiguration.hpp>
#include "ExampleConfig.hpp"

int main(int, char**) {

	auto example = ex::ExampleConfiguration();
	
	std::cout << "Hello World!" << std::endl;
	std::cout << "Root Path: " << ex::ExampleConfig::RootPath << std::endl;

	return EXIT_SUCCESS;
}